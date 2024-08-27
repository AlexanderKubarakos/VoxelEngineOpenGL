#include "ChunkManager.hpp"

#include <algorithm>

#include "imgui.h"

ChunkManager::ChunkManager() : m_DrawPool(6000, 4096 * 4), m_Sorted {false}
{

}

ChunkManager::~ChunkManager() = default;

void ChunkManager::AddChunk(const glm::ivec3& t_ChunkPosition)
{
	m_Chunks.emplace_back(t_ChunkPosition);
	m_MeshingQueue.emplace_back(t_ChunkPosition);
	m_Sorted = false;
}

void ChunkManager::RemoveChunk(const glm::ivec3& t_ChunkPosition)
{
	auto toRemove = GetChunk(t_ChunkPosition);

	if (toRemove == m_Chunks.end())
		return;

	m_DrawPool.FreeBucket(toRemove->m_BucketIDs);

	std::iter_swap(toRemove, --m_Chunks.end());
	m_Chunks.pop_back();
	m_Sorted = false;
}


void ChunkManager::MeshChunks()
{
	while (!m_MeshingQueue.empty())
	{
		MeshChunk(m_MeshingQueue.front());
		m_MeshingQueue.pop_front();
	}
}

void ChunkManager::RenderChunks(const Camera& t_Camera, const glm::mat4& t_Projection)
{
	m_DrawPool.Render(t_Projection * t_Camera.GetViewMatrix(), t_Camera.GetViewMatrix(), t_Camera);
}

void ChunkManager::ShowDebugInfo()
{
	if(ImGui::Button("Re-mesh All Chunks"))
	{
		LOG_PRINT("Re-meshing")
		for (auto& chunk : m_Chunks)
		{
			m_MeshingQueue.push_back(chunk.m_ChunkPosition);
		}
	}
	ImGui::Text("Chunk Count: %i", m_Chunks.size());
	ImGui::Text("Chunk Data Size: %iMB", m_Chunks.size() * 32 * 32 * 32 * sizeof(int8_t)/1024/1024);
	m_DrawPool.Debug();
}

std::vector<Chunk>::iterator ChunkManager::GetChunk(const glm::ivec3& t_ChunkPosition)
{
	if (!m_Sorted)
	{
		std::sort(m_Chunks.begin(), m_Chunks.end(), [](const Chunk& first, const Chunk& last) {
			auto& lh = first.m_ChunkPosition;
			auto& rh = last.m_ChunkPosition;
			return lh.x != rh.x ?
				lh.x < rh.x
				: lh.y != rh.y ?
				lh.y < rh.y
				: lh.z < rh.z; });
		m_Sorted = true;
	}

	auto ab = std::lower_bound(m_Chunks.begin(), m_Chunks.end(), t_ChunkPosition, [](const Chunk& first, const glm::ivec3& rh) {
		auto& lh = first.m_ChunkPosition;
		return lh.x != rh.x ?
			lh.x < rh.x
			: lh.y != rh.y ?
			lh.y < rh.y
			: lh.z < rh.z; });
	
	if (ab != m_Chunks.end() && ab->m_ChunkPosition == t_ChunkPosition)
		return ab;

	return m_Chunks.end();
}

void ChunkManager::MeshChunk(const glm::ivec3& t_ToMesh)
{
	TIMER_START(1)
	auto itr = GetChunk(t_ToMesh);
	if (itr == m_Chunks.end())
		return;

	auto& chunk = *itr;
	auto blockData = chunk.m_BlockData;
	auto& buckets = chunk.m_BucketIDs;

	std::array<int8_t*, 6> neighbors {nullptr};

	auto otherChunk = GetChunk(t_ToMesh + glm::ivec3(0, 1, 0));
	if (otherChunk != m_Chunks.end())
		neighbors[Utilities::UP] = otherChunk->m_BlockData;

	otherChunk = GetChunk(t_ToMesh + glm::ivec3(0, -1, 0));
	if (otherChunk != m_Chunks.end())
		neighbors[Utilities::DOWN] = otherChunk->m_BlockData;

	otherChunk = GetChunk(t_ToMesh + glm::ivec3(0, 0, 1));
	if (otherChunk != m_Chunks.end())
		neighbors[Utilities::SOUTH] = otherChunk->m_BlockData;

	otherChunk = GetChunk(t_ToMesh + glm::ivec3(0, 0, -1));
	if (otherChunk != m_Chunks.end())
		neighbors[Utilities::NORTH] = otherChunk->m_BlockData;

	otherChunk = GetChunk(t_ToMesh + glm::ivec3(1, 0, 0));
	if (otherChunk != m_Chunks.end())
		neighbors[Utilities::EAST] = otherChunk->m_BlockData;

	otherChunk = GetChunk(t_ToMesh + glm::ivec3(-1, 0, 0));
	if (otherChunk != m_Chunks.end())
		neighbors[Utilities::WEST] = otherChunk->m_BlockData;

	std::array<std::vector<Vertex>, 6> vertexData;

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
				// TODO: add check here for if voxels face should be shown, so we skip non visible
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

					//sideAddXY(layerX, layerY, lengthX, lengthY, layerZ);
					Vertex bottomLeft{ layerX, layerY, layerZ };
					Vertex topLeft{ layerX, layerY + lengthY, layerZ };
					Vertex bottomRight{ layerX + lengthX, layerY, layerZ };
					Vertex topRight{ layerX + lengthX, layerY + lengthY, layerZ };

					vertexData[Utilities::DIRECTION::NORTH].push_back(bottomLeft);
					vertexData[Utilities::DIRECTION::NORTH].push_back(topLeft);
					vertexData[Utilities::DIRECTION::NORTH].push_back(bottomRight);
					vertexData[Utilities::DIRECTION::NORTH].push_back(topRight);
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

					//sideAddXY(layerX, layerY, lengthX, lengthY, layerZ + 1);
					int tempZ = layerZ + 1;
					Vertex bottomLeft{ layerX, layerY, tempZ };
					Vertex topLeft{ layerX, layerY + lengthY, tempZ };
					Vertex bottomRight{ layerX + lengthX, layerY, tempZ };
					Vertex topRight{ layerX + lengthX, layerY + lengthY, tempZ };

					vertexData[Utilities::DIRECTION::SOUTH].push_back(bottomLeft);
					vertexData[Utilities::DIRECTION::SOUTH].push_back(topLeft);
					vertexData[Utilities::DIRECTION::SOUTH].push_back(bottomRight);
					vertexData[Utilities::DIRECTION::SOUTH].push_back(topRight);
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
					int tempY = layerY + 1;
					Vertex bottomLeft{ layerX, tempY, layerZ };
					Vertex topLeft{ layerX, tempY, layerZ + lengthZ };
					Vertex bottomRight{ layerX + lengthX, tempY, layerZ };
					Vertex topRight{ layerX + lengthX, tempY, layerZ + lengthZ };

					vertexData[Utilities::DIRECTION::UP].push_back(bottomLeft);
					vertexData[Utilities::DIRECTION::UP].push_back(topLeft);
					vertexData[Utilities::DIRECTION::UP].push_back(bottomRight);
					vertexData[Utilities::DIRECTION::UP].push_back(topRight);
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

					//sideAddXZ(layerX, layerZ, lengthX, lengthZ, layerY);
					Vertex bottomLeft{ layerX, layerY, layerZ };
					Vertex topLeft{ layerX, layerY, layerZ + lengthZ };
					Vertex bottomRight{ layerX + lengthX, layerY, layerZ };
					Vertex topRight{ layerX + lengthX, layerY, layerZ + lengthZ };

					vertexData[Utilities::DIRECTION::DOWN].push_back(bottomLeft);
					vertexData[Utilities::DIRECTION::DOWN].push_back(topLeft);
					vertexData[Utilities::DIRECTION::DOWN].push_back(bottomRight);
					vertexData[Utilities::DIRECTION::DOWN].push_back(topRight);
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
					int tempX = layerX + 1;
					Vertex bottomLeft{ tempX, layerY, layerZ };
					Vertex topLeft{ tempX, layerY, layerZ + lengthZ };
					Vertex bottomRight{ tempX, layerY + lengthY, layerZ };
					Vertex topRight{ tempX, layerY + lengthY, layerZ + lengthZ };

					vertexData[Utilities::DIRECTION::EAST].push_back(bottomLeft);
					vertexData[Utilities::DIRECTION::EAST].push_back(topLeft);
					vertexData[Utilities::DIRECTION::EAST].push_back(bottomRight);
					vertexData[Utilities::DIRECTION::EAST].push_back(topRight);
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

					//sideAddYZ(layerY, layerZ, lengthY, lengthZ, layerX);
					Vertex bottomLeft{ layerX, layerY, layerZ };
					Vertex topLeft{ layerX, layerY, layerZ + lengthZ };
					Vertex bottomRight{ layerX, layerY + lengthY, layerZ };
					Vertex topRight{ layerX, layerY + lengthY, layerZ + lengthZ };

					vertexData[Utilities::DIRECTION::WEST].push_back(bottomLeft);
					vertexData[Utilities::DIRECTION::WEST].push_back(topLeft);
					vertexData[Utilities::DIRECTION::WEST].push_back(bottomRight);
					vertexData[Utilities::DIRECTION::WEST].push_back(topRight);
				}
			}
		}
	}
	int count = 0;
	bool notBlankChunk = false;
	for (int i = 0; i < 6; i++)
	{
		Utilities::DIRECTION direction = static_cast<Utilities::DIRECTION>(i);
		if (vertexData[direction].empty())
			continue;

		if (buckets[direction] == nullptr)
			buckets[direction] = m_DrawPool.AllocateBucket(static_cast<int>(vertexData[direction].size()));

		count += vertexData[direction].size();
		notBlankChunk = true;
		auto extraData = glm::ivec4(t_ToMesh, 0);
		m_DrawPool.FillBucket(buckets[direction], vertexData[direction], direction, extraData);
	}

	// Only print time if not trivial chunk
	if (notBlankChunk)
	{
		TIMER_END(1, "Meshing time: ")
	}
}
