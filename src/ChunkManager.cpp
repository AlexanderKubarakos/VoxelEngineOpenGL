#include "ChunkManager.hpp"

#include "imgui.h"
#include <set>

#include <common/TracySystem.hpp>

#include "Tracy.hpp"

std::atomic_bool stopToken{ false };
std::atomic_bool signal{ true };
ChunkManager::ChunkManager() : m_DrawPool(1024 * 32, 4096 * 2), m_LoadUnloadChunks{ true }, m_ViewDistance {12},
m_ChunksToRemove{4096*16}, m_ChunksToAdd{ 4096 * 16 }, m_ChunksToMesh{ 4096 * 16 }, m_MeshDataToProcesses{4096*16}
{

}

ChunkManager::~ChunkManager()
{
	stopToken = true;
	m_Thread.join();
	m_ThreadMeshing.join();
}

static bool first = true;
void ChunkManager::LoadUnloadAroundPlayer(const Camera& camera)
{
	if (!m_LoadUnloadChunks)
		return;
	if (first)
	{
		m_Thread = std::thread { &ChunkManager::ThreadedUnloadAndLoad, this, std::ref(camera) };
		m_ThreadMeshing = std::thread{ &ChunkManager::ThreadedMeshing, this };
		first = false;
	}
	else
	{
		ProcessChunks(camera.GetAtomicCameraPos());
	}
}

void ChunkManager::ThreadedUnloadAndLoad(const Camera& camera)
{
	tracy::SetThreadName("Chunk Loading/Unloading");
	while (true)
	{
		std::shared_lock lock { m_Mutex, std::defer_lock };

		while (!signal && !stopToken);
		signal = false;
		while (!m_ChunksToAdd.empty() || !m_ChunksToRemove.empty() && !stopToken);
		lock.lock();
		if (stopToken)
			break;
		{
			ZoneScoped;
			glm::vec3 cameraPos = camera.GetAtomicCameraPos();
			const glm::ivec3 playerPositionChunkSpace = static_cast<glm::ivec3>(cameraPos) / 32;
			int leftXBound = playerPositionChunkSpace.x - m_ViewDistance;
			int rightXBound = playerPositionChunkSpace.x + m_ViewDistance;
			int farZBound = playerPositionChunkSpace.z - m_ViewDistance;
			int closeZBound = playerPositionChunkSpace.z + m_ViewDistance;

			std::deque<glm::ivec3> removeQueue;
			std::deque<glm::ivec3> loadQueue;
			auto inViewRange = [=](const auto& item)
				{
					auto const& [key, chunk] = item;
					return key.x < leftXBound || key.x > rightXBound
						|| key.z < farZBound || key.z > closeZBound;
				};

			for (auto first = m_Chunks.begin(), last = m_Chunks.end(); first != last;)
			{
				if (inViewRange(*first))
				{
					removeQueue.push_back(first->first);
				}
				++first;
			}

			// TODO: Much more efficient way is to only do this when we go between chunks, and even then we can see what direction we moved in and only load that "sides" chunks
			for (int x = -m_ViewDistance + playerPositionChunkSpace.x; x <= m_ViewDistance + playerPositionChunkSpace.x; x++)
				for (int z = -m_ViewDistance + playerPositionChunkSpace.z; z <= m_ViewDistance + playerPositionChunkSpace.z; z++)
					for (int y = -3; y <= 3; y++)
					{
						glm::ivec3 chunkPos{ x, y, z };
						// TODO: maybe get rid of max size?
						if (!m_Chunks.contains(chunkPos))
						{
							loadQueue.emplace_back(chunkPos);
						}
					}
			if (loadQueue.size() == 2048)
				signal = true;
			lock.unlock();

			for (auto& i : removeQueue)
				m_ChunksToRemove.push(i);

			for (auto& i : loadQueue)
			{
				auto ptr = std::make_shared<Chunk>(i);
				m_ChunksToAdd.push(ptr);
				// TODO: maybe an arena allocator
			}
		}
		
	}
	stopToken = false;
}
glm::ivec3 previousChunk = { 0,0,-999999 };
void ChunkManager::ProcessChunks(const glm::vec3& t_PlayerPosition)
{
	ZoneScoped;
	std::unique_lock lock{ m_Mutex, std::defer_lock };
	if (previousChunk != glm::ivec3(t_PlayerPosition / 32.0f))
	{
		LOG_PRINT("Changing chunk...")
		previousChunk = glm::ivec3(t_PlayerPosition / 32.0f);
		signal = true;
	}
	// 1. Make any changes to the Hashmap, add new chunks, remove old chunks, checking if all threads are finished first, if not skip to next frame
	if ((!m_ChunksToRemove.empty() || !m_ChunksToAdd.empty()) && lock.try_lock())
	{
		while (!m_ChunksToRemove.empty())
		{
			auto& bucket = m_Chunks.at(m_ChunksToRemove.peek())->m_BucketIDs;
			m_DrawPool.FreeBucket(bucket);
			m_Chunks.erase(m_ChunksToRemove.peek());
			m_ChunksToRemove.pop();
		}

		while (!m_ChunksToAdd.empty())
		{
			m_Chunks.emplace(m_ChunksToAdd.peek()->m_ChunkPosition, m_ChunksToAdd.peek());
			m_ChunksToMesh.push(m_ChunksToAdd.peek()->m_ChunkPosition);
			m_ChunksToAdd.pop();
		}
		lock.unlock();
	}

	while (!m_MeshDataToProcesses.empty())
	{
		auto data = m_MeshDataToProcesses.pop();
		auto itr = m_Chunks.find(data.m_Chunk);
		if (itr == m_Chunks.end())
			continue;
		auto& buckets = itr->second->m_BucketIDs;
		auto& faceData = data.m_FaceData;

		// Why free the buckets first??
		m_DrawPool.FreeBucket(buckets);

		for (int i = 0; i < 6; i++)
		{
			Utilities::DIRECTION direction = static_cast<Utilities::DIRECTION>(i);
			if (faceData[direction].empty())
				continue;

			buckets[direction] = m_DrawPool.AllocateBucket(static_cast<int>(faceData[direction].size()));

			auto extraData = glm::ivec4(data.m_Chunk, 0);
			m_DrawPool.FillBucket(buckets[direction], faceData[direction], direction, extraData);
		}
	}
}

void ChunkManager::RenderChunks(const Camera& t_Camera, const glm::mat4& t_Projection)
{
	ZoneScoped;
	m_DrawPool.Render(t_Projection * t_Camera.GetViewMatrix(), t_Camera.GetViewMatrix(), t_Camera);
}

void ChunkManager::ShowDebugInfo()
{
	ZoneScoped;
	ImGui::Checkbox("Load and Unload Chunks Around Player", &m_LoadUnloadChunks);
	if (ImGui::SliderInt("Render Distance", &m_ViewDistance, 1, 32))
	{
		signal = true;
	}
		
	if(ImGui::Button("Re-mesh All Chunks"))
	{
		LOG_PRINT("Re-meshing")
		for (auto& chunk : m_Chunks)
		{
			m_ChunksToMesh.push(chunk.second->m_ChunkPosition);
		}
	}
	ImGui::Text("Chunk Count: %i", m_Chunks.size());
	ImGui::Text("Chunk Data Size: %iMB", m_Chunks.size() * 32 * 32 * 32 * sizeof(int8_t)/1024/1024);
	m_DrawPool.Debug();
}
//TODO: add per side re meshing, aka if a chunk on this chunks top boarder loads, we only need to redo that bucket

void ChunkManager::ThreadedMeshing()
{
	tracy::SetThreadName("Meshing");
	// TODO: dont allow meshing to hog the mutex, we need to dump our loaded chunk data to main thread
	// since we hog the shared mutex, process chunk almost never runs, and loading chunks cant continue
	while (!stopToken)
	{
		while (!m_ChunksToMesh.empty())
		{
			ZoneScoped;
			std::shared_lock lock{ m_Mutex};
			
			auto t_ToMesh = m_ChunksToMesh.pop();
			auto itr = m_Chunks.find(t_ToMesh);
			if (itr == m_Chunks.end())
				return;

			auto& chunk = *itr;
			auto blockData = chunk.second->m_BlockData;

			if (chunk.second->allAir)
				continue;

			std::array<int8_t*, 6> neighbors{ nullptr };

			auto otherChunk = m_Chunks.find(t_ToMesh + glm::ivec3(0, 1, 0));
			if (otherChunk != m_Chunks.end())
				neighbors[Utilities::UP] = otherChunk->second->m_BlockData;

			otherChunk = m_Chunks.find(t_ToMesh + glm::ivec3(0, -1, 0));
			if (otherChunk != m_Chunks.end())
				neighbors[Utilities::DOWN] = otherChunk->second->m_BlockData;

			otherChunk = m_Chunks.find(t_ToMesh + glm::ivec3(0, 0, 1));
			if (otherChunk != m_Chunks.end())
				neighbors[Utilities::SOUTH] = otherChunk->second->m_BlockData;

			otherChunk = m_Chunks.find(t_ToMesh + glm::ivec3(0, 0, -1));
			if (otherChunk != m_Chunks.end())
				neighbors[Utilities::NORTH] = otherChunk->second->m_BlockData;

			otherChunk = m_Chunks.find(t_ToMesh + glm::ivec3(1, 0, 0));
			if (otherChunk != m_Chunks.end())
				neighbors[Utilities::EAST] = otherChunk->second->m_BlockData;

			otherChunk = m_Chunks.find(t_ToMesh + glm::ivec3(-1, 0, 0));
			if (otherChunk != m_Chunks.end())
				neighbors[Utilities::WEST] = otherChunk->second->m_BlockData;
			std::array<std::vector<FaceVertex>, 6> faceData;

			// xy plane, facing -layerZ
			for (int layerZ = 0; layerZ < 32; layerZ++)
			{
				//TODO: get pointer to m_Block data for [0, 0, layerZ] so that we can access with less cache misses
				int32_t bitmap[32] = { 0 }; // xyBitmap[0] is x row of y = 0, xyBitmap[1] is x row of y = 1
				for (int layerY = 0; layerY < 32; layerY++)
				{
					for (int layerX = 0; layerX < 32; layerX++)
					{
						// find a block, that hasn't been meshed yet,
						// TODO: add check here for if voxels face should be shown, so we skip non-visible
						if (blockData[layerX + layerY * 32 + layerZ * 32 * 32] > 0 && !((bitmap[layerY] >> layerX) & 0x1))
						{
							// TODO: maybe move this to upper if statement
							if (layerZ != 0 && blockData[layerX + layerY * 32 + (layerZ - 1) * 32 * 32] != 0)
								continue;

							if (layerZ == 0 && (!neighbors[Utilities::NORTH] || neighbors[Utilities::NORTH][layerX + layerY * 32 + 31 * 32 * 32] != 0))
								continue;

							// Looking at voxel at [layerX, layerY]
							int voxelType = blockData[layerX + layerY * 32 + layerZ * 32 * 32];
							//TODO: do optimization by moving layerX by lengthX, just an easy skip, remember that we layerX++, but maybe that should be removed?

							// mark the current voxel
							bitmap[layerY] |= 0x1 << layerX;

							// expand mesh to the +x direction
							int lengthX = 1; // length of side, default 1 since it's minimum one voxel
							for (int subX = layerX + 1; subX < 32; subX++)
							{
								// TODO
								// if (subY != 31 && subX != 31 && blockData[(subX + 1) + (layerY + 1) * 32 + layerZ * 32 * 32]
								// expand till we reach a voxel of different type - or air
								// these are our stops conditions, if next voxel isn't correct type, if voxel has already been explored, and if both pass
								// then check if next side should be visible

								if (blockData[subX + layerY * 32 + layerZ * 32 * 32] != voxelType // this part of row is not same voxel
									|| (bitmap[layerY] >> subX) & 0x1 // this part of row is already in a mesh
									|| (layerZ != 0 && blockData[subX + layerY * 32 + (layerZ - 1) * 32 * 32] != 0) // this parts side is blocked off
									|| (layerZ == 0 && neighbors[Utilities::NORTH][subX + layerY * 32 + 31 * 32 * 32] != 0))
								{
									lengthX = subX - layerX;
									break;
								}

								bitmap[layerY] |= 0x1 << subX;

								// if we have reached the last voxel on the x axis t hen 
								if (subX == 31)
								{
									lengthX = subX - layerX + 1;
									break;
								}
							}

							// try to expand to the +y direction
							int lengthY = 1;
							for (int subY = layerY + 1; subY < 32; subY++)
							{
								bool completeRow = true;
								for (int subX = layerX; subX < layerX + lengthX; subX++)
								{
									// demorgans law here to allow short circuit might help
									if (blockData[subX + subY * 32 + layerZ * 32 * 32] != voxelType
										|| ((bitmap[subY] >> subX) & 0x1)
										|| (layerZ != 0 && blockData[subX + subY * 32 + (layerZ - 1) * 32 * 32] != 0)
										|| (layerZ == 0 && neighbors[Utilities::NORTH][subX + subY * 32 + 31 * 32 * 32] != 0))
									{
										completeRow = false;
										break;
									}
								}

								if (!completeRow)
									break;

								lengthY += 1;

								// else continue looping and mark whole row in bit map
								int32_t voxelMask = 0x1 << layerX;
								for (int i = layerX; i < layerX + lengthX; i++)
								{
									voxelMask |= 0x1 << i;
								}
								bitmap[subY] |= voxelMask;
							}

							FaceVertex face{ layerX, layerY, layerZ, lengthX, lengthY };

							faceData[Utilities::DIRECTION::NORTH].push_back(face);
						}
					}
				}
			}

			// xy plane, facing +layerZ
			for (int layerZ = 0; layerZ < 32; layerZ++)
			{
				int32_t bitmap[32] = { 0 };
				for (int layerY = 0; layerY < 32; layerY++)
				{
					for (int layerX = 0; layerX < 32; layerX++)
					{
						if (blockData[layerX + layerY * 32 + layerZ * 32 * 32] > 0 && !((bitmap[layerY] >> layerX) & 0x1))
						{
							if (layerZ != 31 && blockData[layerX + layerY * 32 + (layerZ + 1) * 32 * 32] != 0)
								continue;

							if (layerZ == 31 && (!neighbors[Utilities::SOUTH] || neighbors[Utilities::SOUTH][layerX + layerY * 32 + 0 * 32 * 32] != 0))
								continue;

							int voxelType = blockData[layerX + layerY * 32 + layerZ * 32 * 32];

							bitmap[layerY] |= 0x1 << layerX;

							int lengthX = 1;
							for (int subX = layerX + 1; subX < 32; subX++)
							{
								if (blockData[subX + layerY * 32 + layerZ * 32 * 32] != voxelType
									|| (bitmap[layerY] >> subX) & 0x1
									|| (layerZ != 31 && blockData[subX + layerY * 32 + (layerZ + 1) * 32 * 32] != 0)
									|| (layerZ == 31 && neighbors[Utilities::SOUTH][subX + layerY * 32 + 0 * 32 * 32] != 0))
								{
									lengthX = subX - layerX;
									break;
								}

								bitmap[layerY] |= 0x1 << subX;

								if (subX == 31)
								{
									lengthX = subX - layerX + 1;
									break;
								}
							}

							int lengthY = 1;
							for (int subY = layerY + 1; subY < 32; subY++)
							{
								bool completeRow = true;
								for (int subX = layerX; subX < layerX + lengthX; subX++)
								{
									if (blockData[subX + subY * 32 + layerZ * 32 * 32] != voxelType
										|| ((bitmap[subY] >> subX) & 0x1)
										|| (layerZ != 31 && blockData[subX + subY * 32 + (layerZ + 1) * 32 * 32] != 0)
										|| (layerZ == 31 && neighbors[Utilities::SOUTH][subX + subY * 32 + 0 * 32 * 32] != 0))
									{
										completeRow = false;
										break;
									}
								}

								if (!completeRow)
									break;

								lengthY += 1;

								int32_t voxelMask = 0x1 << layerX;
								for (int i = layerX; i < layerX + lengthX; i++)
								{
									voxelMask |= 0x1 << i;
								}
								bitmap[subY] |= voxelMask;
							}

							FaceVertex face{ layerX, layerY, layerZ, lengthX, lengthY };

							faceData[Utilities::DIRECTION::SOUTH].push_back(face);
						}
					}
				}
			}

			// xz plane, facing +y
			for (int layerY = 0; layerY < 32; layerY++)
			{
				int32_t bitmap[32] = { 0 };
				for (int layerZ = 0; layerZ < 32; layerZ++)
				{
					for (int layerX = 0; layerX < 32; layerX++)
					{
						if (blockData[layerX + layerY * 32 + layerZ * 32 * 32] > 0 && !((bitmap[layerZ] >> layerX) & 0x1))
						{
							if (layerY != 31 && blockData[layerX + (layerY + 1) * 32 + layerZ * 32 * 32] != 0)
								continue;

							if (layerY == 31 && (!neighbors[Utilities::UP] || neighbors[Utilities::UP][layerX + 0 * 32 + layerZ * 32 * 32] != 0))
								continue;

							int voxelType = blockData[layerX + layerY * 32 + layerZ * 32 * 32];

							bitmap[layerZ] |= 0x1 << layerX;

							int lengthX = 1;
							for (int subX = layerX + 1; subX < 32; subX++)
							{
								if (blockData[subX + layerY * 32 + layerZ * 32 * 32] != voxelType
									|| (bitmap[layerZ] >> subX) & 0x1
									|| (layerY != 31 && blockData[subX + (layerY + 1) * 32 + layerZ * 32 * 32] != 0)
									|| (layerY == 31 && neighbors[Utilities::UP][subX + 0 * 32 + layerZ * 32 * 32] != 0))
								{
									lengthX = subX - layerX;
									break;
								}

								bitmap[layerZ] |= 0x1 << subX;

								if (subX == 31)
								{
									lengthX = subX - layerX + 1;
									break;
								}
							}

							int lengthZ = 1;
							for (int subZ = layerZ + 1; subZ < 32; subZ++)
							{
								bool completeRow = true;
								for (int subX = layerX; subX < layerX + lengthX; subX++)
								{
									if (blockData[subX + layerY * 32 + subZ * 32 * 32] != voxelType
										|| ((bitmap[subZ] >> subX) & 0x1)
										|| (layerY != 31 && blockData[subX + (layerY + 1) * 32 + subZ * 32 * 32] != 0)
										|| (layerY == 31 && neighbors[Utilities::UP][subX + 0 * 32 + subZ * 32 * 32] != 0))
									{
										completeRow = false;
										break;
									}
								}

								if (!completeRow)
									break;

								lengthZ += 1;

								int32_t voxelMask = 0x1 << layerX;
								for (int i = layerX; i < layerX + lengthX; i++)
								{
									voxelMask |= 0x1 << i;
								}
								bitmap[subZ] |= voxelMask;
							}

							//sideAddXZ(layerX, layerZ, lengthX, lengthZ, layerY + 1);
							FaceVertex face{ layerX, layerY, layerZ, lengthX, lengthZ };

							faceData[Utilities::DIRECTION::UP].push_back(face);
						}
					}
				}
			}

			// xz plane, facing -y
			for (int layerY = 0; layerY < 32; layerY++)
			{
				int32_t bitmap[32] = { 0 };
				for (int layerZ = 0; layerZ < 32; layerZ++)
				{
					for (int layerX = 0; layerX < 32; layerX++)
					{
						if (blockData[layerX + layerY * 32 + layerZ * 32 * 32] > 0 && !((bitmap[layerZ] >> layerX) & 0x1))
						{
							if (layerY != 0 && blockData[layerX + (layerY - 1) * 32 + layerZ * 32 * 32] != 0)
								continue;

							if (layerY == 0 && (!neighbors[Utilities::DOWN] || neighbors[Utilities::DOWN][layerX + 31 * 32 + layerZ * 32 * 32] != 0))
								continue;

							int voxelType = blockData[layerX + layerY * 32 + layerZ * 32 * 32];

							bitmap[layerZ] |= 0x1 << layerX;

							int lengthX = 1;
							for (int subX = layerX + 1; subX < 32; subX++)
							{
								if (blockData[subX + layerY * 32 + layerZ * 32 * 32] != voxelType
									|| (bitmap[layerZ] >> subX) & 0x1
									|| (layerY != 0 && blockData[subX + (layerY - 1) * 32 + layerZ * 32 * 32] != 0)
									|| (layerY == 0 && neighbors[Utilities::DOWN][subX + 31 * 32 + layerZ * 32 * 32] != 0))
								{
									lengthX = subX - layerX;
									break;
								}

								bitmap[layerZ] |= 0x1 << subX;

								if (subX == 31)
								{
									lengthX = subX - layerX + 1;
									break;
								}
							}

							int lengthZ = 1;
							for (int subZ = layerZ + 1; subZ < 32; subZ++)
							{
								bool completeRow = true;
								for (int subX = layerX; subX < layerX + lengthX; subX++)
								{
									if (blockData[subX + layerY * 32 + subZ * 32 * 32] != voxelType
										|| ((bitmap[subZ] >> subX) & 0x1)
										|| (layerY != 0 && blockData[subX + (layerY - 1) * 32 + subZ * 32 * 32] != 0)
										|| (layerY == 0 && neighbors[Utilities::DOWN][subX + 31 * 32 + subZ * 32 * 32] != 0))
									{
										completeRow = false;
										break;
									}
								}

								if (!completeRow)
									break;

								lengthZ += 1;

								int32_t voxelMask = 0x1 << layerX;
								for (int i = layerX; i < layerX + lengthX; i++)
								{
									voxelMask |= 0x1 << i;
								}
								bitmap[subZ] |= voxelMask;
							}

							FaceVertex face{ layerX, layerY, layerZ, lengthX, lengthZ };

							faceData[Utilities::DIRECTION::DOWN].push_back(face);
						}
					}
				}
			}

			// yz facing +x
			for (int layerX = 0; layerX < 32; layerX++)
			{
				int32_t bitmap[32] = { 0 };
				for (int layerZ = 0; layerZ < 32; layerZ++)
				{
					for (int layerY = 0; layerY < 32; layerY++)
					{
						if (blockData[layerX + layerY * 32 + layerZ * 32 * 32] > 0 && !((bitmap[layerZ] >> layerY) & 0x1))
						{
							if (layerX != 31 && blockData[(layerX + 1) + layerY * 32 + layerZ * 32 * 32] != 0)
								continue;

							if (layerX == 31 && (!neighbors[Utilities::EAST] || neighbors[Utilities::EAST][0 + layerY * 32 + layerZ * 32 * 32] != 0))
								continue;

							int voxelType = blockData[layerX + layerY * 32 + layerZ * 32 * 32];

							bitmap[layerZ] |= 0x1 << layerY;

							int lengthY = 1;
							for (int subY = layerY + 1; subY < 32; subY++)
							{
								if (blockData[layerX + subY * 32 + layerZ * 32 * 32] != voxelType
									|| (bitmap[layerZ] >> subY) & 0x1
									|| (layerX != 31 && blockData[(layerX + 1) + subY * 32 + layerZ * 32 * 32] != 0)
									|| (layerX == 31 && neighbors[Utilities::EAST][0 + subY * 32 + layerZ * 32 * 32] != 0))
								{
									lengthY = subY - layerY;
									break;
								}

								bitmap[layerZ] |= 0x1 << subY;

								if (subY == 31)
								{
									lengthY = subY - layerY + 1;
									break;
								}
							}

							int lengthZ = 1;
							for (int subZ = layerZ + 1; subZ < 32; subZ++)
							{
								bool completeRow = true;
								for (int subY = layerY; subY < layerY + lengthY; subY++)
								{
									if (blockData[layerX + subY * 32 + subZ * 32 * 32] != voxelType
										|| ((bitmap[subZ] >> subY) & 0x1)
										|| (layerX != 31 && blockData[(layerX + 1) + subY * 32 + subZ * 32 * 32] != 0)
										|| (layerX == 31 && neighbors[Utilities::EAST][0 + subY * 32 + subZ * 32 * 32] != 0))
									{
										completeRow = false;
										break;
									}
								}

								if (!completeRow)
									break;

								lengthZ += 1;

								int32_t voxelMask = 0x1 << layerY;
								for (int i = layerY; i < layerY + lengthY; i++)
								{
									voxelMask |= 0x1 << i;
								}
								bitmap[subZ] |= voxelMask;
							}

							//sideAddYZ(layerY, layerZ, lengthY, lengthZ, layerX + 1);
							FaceVertex face{ layerX, layerY, layerZ, lengthY, lengthZ };

							faceData[Utilities::DIRECTION::EAST].push_back(face);
						}
					}
				}
			}

			// yz facing -x
			for (int layerX = 0; layerX < 32; layerX++)
			{
				int32_t bitmap[32] = { 0 };
				for (int layerZ = 0; layerZ < 32; layerZ++)
				{
					for (int layerY = 0; layerY < 32; layerY++)
					{
						if (blockData[layerX + layerY * 32 + layerZ * 32 * 32] > 0 && !((bitmap[layerZ] >> layerY) & 0x1))
						{
							if (layerX != 0 && blockData[(layerX - 1) + layerY * 32 + layerZ * 32 * 32] != 0)
								continue;

							if (layerX == 0 && (!neighbors[Utilities::WEST] || neighbors[Utilities::WEST][31 + layerY * 32 + layerZ * 32 * 32] != 0))
								continue;

							int voxelType = blockData[layerX + layerY * 32 + layerZ * 32 * 32];

							bitmap[layerZ] |= 0x1 << layerY;

							int lengthY = 1;
							for (int subY = layerY + 1; subY < 32; subY++)
							{
								if (blockData[layerX + subY * 32 + layerZ * 32 * 32] != voxelType
									|| (bitmap[layerZ] >> subY) & 0x1
									|| (layerX != 0 && blockData[(layerX - 1) + subY * 32 + layerZ * 32 * 32] != 0)
									|| (layerX == 0 && neighbors[Utilities::WEST][31 + subY * 32 + layerZ * 32 * 32] != 0))
								{
									lengthY = subY - layerY;
									break;
								}

								bitmap[layerZ] |= 0x1 << subY;

								if (subY == 31)
								{
									lengthY = subY - layerY + 1;
									break;
								}
							}

							int lengthZ = 1;
							for (int subZ = layerZ + 1; subZ < 32; subZ++)
							{
								bool completeRow = true;
								for (int subY = layerY; subY < layerY + lengthY; subY++)
								{
									if (blockData[layerX + subY * 32 + subZ * 32 * 32] != voxelType
										|| ((bitmap[subZ] >> subY) & 0x1)
										|| (layerX != 0 && blockData[(layerX - 1) + subY * 32 + subZ * 32 * 32] != 0)
										|| (layerX == 0 && neighbors[Utilities::WEST][31 + subY * 32 + subZ * 32 * 32] != 0))
									{
										completeRow = false;
										break;
									}
								}

								if (!completeRow)
									break;

								lengthZ += 1;

								int32_t voxelMask = 0x1 << layerY;
								for (int i = layerY; i < layerY + lengthY; i++)
								{
									voxelMask |= 0x1 << i;
								}
								bitmap[subZ] |= voxelMask;
							}

							FaceVertex face{ layerX, layerY, layerZ, lengthY, lengthZ };

							faceData[Utilities::DIRECTION::WEST].push_back(face);
						}
					}
				}
			}

			m_MeshDataToProcesses.push({ faceData, t_ToMesh });
		}
	}


}

//void ChunkManager::MeshUp(int8_t* t_NeighborData, std::vector<FaceVertex>& t_Data, const glm::ivec3& t_ToMesh)
//{
//	auto itr = m_Chunks.find(t_ToMesh);
//	if (itr == m_Chunks.end())
//		return;
//
//	auto& chunk = *itr;
//	auto blockData = chunk.second->m_BlockData;
//	for (int layerY = 0; layerY < 32; layerY++)
//	{
//		int32_t bitmap[32] = { 0 };
//		for (int layerZ = 0; layerZ < 32; layerZ++)
//		{
//			for (int layerX = 0; layerX < 32; layerX++)
//			{
//				if (blockData[layerX + layerY * 32 + layerZ * 32 * 32] > 0 && !((bitmap[layerZ] >> layerX) & 0x1))
//				{
//					if (layerY != 31 && blockData[layerX + (layerY + 1) * 32 + layerZ * 32 * 32] != 0)
//						continue;
//
//					if (layerY == 31 && t_NeighborData[layerX + 0 * 32 + layerZ * 32 * 32] != 0)
//						continue;
//
//					int voxelType = blockData[layerX + layerY * 32 + layerZ * 32 * 32];
//
//					bitmap[layerZ] |= 0x1 << layerX;
//
//					int lengthX = 1;
//					for (int subX = layerX + 1; subX < 32; subX++)
//					{
//						if (blockData[subX + layerY * 32 + layerZ * 32 * 32] != voxelType
//							|| (bitmap[layerZ] >> subX) & 0x1
//							|| (layerY != 31 && blockData[subX + (layerY + 1) * 32 + layerZ * 32 * 32] != 0)
//							|| (layerY == 31 && t_NeighborData[subX + 0 * 32 + layerZ * 32 * 32] != 0))
//						{
//							lengthX = subX - layerX;
//							break;
//						}
//
//						bitmap[layerZ] |= 0x1 << subX;
//
//						if (subX == 31)
//						{
//							lengthX = subX - layerX + 1;
//							break;
//						}
//					}
//
//					int lengthZ = 1;
//					for (int subZ = layerZ + 1; subZ < 32; subZ++)
//					{
//						bool completeRow = true;
//						for (int subX = layerX; subX < layerX + lengthX; subX++)
//						{
//							if (blockData[subX + layerY * 32 + subZ * 32 * 32] != voxelType
//								|| ((bitmap[subZ] >> subX) & 0x1)
//								|| (layerY != 31 && blockData[subX + (layerY + 1) * 32 + subZ * 32 * 32] != 0)
//								|| (layerY == 31 && t_NeighborData[subX + 0 * 32 + subZ * 32 * 32] != 0))
//							{
//								completeRow = false;
//								break;
//							}
//						}
//
//						if (!completeRow)
//							break;
//
//						lengthZ += 1;
//
//						int32_t voxelMask = 0x1 << layerX;
//						for (int i = layerX; i < layerX + lengthX; i++)
//						{
//							voxelMask |= 0x1 << i;
//						}
//						bitmap[subZ] |= voxelMask;
//					}
//
//					//sideAddXZ(layerX, layerZ, lengthX, lengthZ, layerY + 1);
//					FaceVertex face{ layerX, layerY, layerZ, lengthX, lengthZ };
//
//					t_Data.push_back(face);
//				}
//			}
//		}
//	}
//}
//
//void ChunkManager::MeshDown(int8_t* t_NeighborData, std::vector<FaceVertex>& t_Data, const glm::ivec3& t_ToMesh)
//{
//}
//
//void ChunkManager::MeshSouth(int8_t* t_NeighborData, std::vector<FaceVertex>& t_Data, const glm::ivec3& t_ToMesh)
//{
//}
//
//void ChunkManager::MeshNorth(int8_t* t_NeighborData, std::vector<FaceVertex>& t_Data, const glm::ivec3& t_ToMesh)
//{
//}
//
//void ChunkManager::MeshEast(int8_t* t_NeighborData, std::vector<FaceVertex>& t_Data, const glm::ivec3& t_ToMesh)
//{
//}
//
//void ChunkManager::MeshWest(int8_t* t_NeighborData, std::vector<FaceVertex>& t_Data, const glm::ivec3& t_ToMesh)
//{
//}
