#include "ChunkManager.hpp"

#include <algorithm>


ChunkManager::ChunkManager() : m_DrawPool(8192, 4096), m_Sorted {false}
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
	m_DrawPool.Render(t_Projection * t_Camera.GetViewMatrix(), t_Camera.GetViewMatrix());
}

void ChunkManager::ShowDebugInfo()
{
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

	return std::lower_bound(m_Chunks.begin(), m_Chunks.end(), t_ChunkPosition, [](const Chunk& first, const glm::ivec3& rh) {
		auto& lh = first.m_ChunkPosition;
		return lh.x != rh.x ?
			lh.x < rh.x
			: lh.y != rh.y ?
			lh.y < rh.y
			: lh.z < rh.z; });

	auto chunkComparator = [t_ChunkPosition](const Chunk& t_Chunk) { return t_ChunkPosition == t_Chunk.m_ChunkPosition; };
	return std::ranges::find_if(m_Chunks.begin(), m_Chunks.end(), chunkComparator);
}

void ChunkManager::MeshChunk(const glm::ivec3& t_ToMesh)
{
	TIMER_START(1)
	auto itr = GetChunk(t_ToMesh);
	if (itr == m_Chunks.end())
		return;

	auto& chunk = *itr;
	auto& blockData = chunk.m_BlockData;

	std::array<std::shared_ptr<int8_t[]>, 6> neighbors;

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
	

	for (auto id : chunk.m_BucketIDs)
	{
		if (id != nullptr)
		{
			m_DrawPool.FreeBucket(id);
		}
	}

	std::array<std::vector<Vertex>, 6> vertexData;

	// xy plane, facing -layerZ
	for (int layerZ = 0; layerZ < 16; layerZ++)
	{
		//TODO: get pointer to m_Block data for [0, 0, layerZ] so that we can access with less cache misses
		int16_t bitmap[16] = { 0 }; // xyBitmap[0] is x row of y = 0, xyBitmap[1] is x row of y = 1
		for (int layerY = 0; layerY < 16; layerY++)
		{
			for (int layerX = 0; layerX < 16; layerX++)
			{
				// find a block, that hasn't been meshed yet,
				// TODO: add check here for if voxels face should be shown, so we skip non visible
				if (blockData[layerX + layerY * 16 + layerZ * 16 * 16] > 0 && !((bitmap[layerY] >> layerX) & 0x1))
				{
					// TODO: maybe move this to upper if statement
					if (layerZ != 0 && blockData[layerX + layerY * 16 + (layerZ - 1) * 16 * 16] != 0)
						continue;

					// maybe mark it on the bitmap? to speed up later code
					if (layerZ == 0 && neighbors[Utilities::NORTH] && neighbors[Utilities::NORTH][layerX + layerY * 16 + 15 * 16 * 16] != 0)
						continue;

					// Looking at voxel at [layerX, layerY]
					int voxelType = blockData[layerX + layerY * 16 + layerZ * 16 * 16];
					//TODO: do optimization by moving layerX by lengthX, just an easy skip, remember that we layerX++, but maybe that should be removed?

					// mark the current voxel
					bitmap[layerY] |= 0x1 << layerX;

					// expand mesh to the +x direction
					int lengthX = 1; // length of side, default 1 since it's minimum one voxel
					for (int subX = layerX + 1; subX < 16; subX++)
					{
						// TODO
						// if (subY != 15 && subX != 15 && blockData[(subX + 1) + (layerY + 1) * 16 + layerZ * 16 * 16]
						// expand till we reach a voxel of different type - or air
						// these are our stops conditions, if next voxel isn't correct type, if voxel has already been explored, and if both pass
						// then check if next side should be visible

						if (blockData[subX + layerY * 16 + layerZ * 16 * 16] != voxelType // this part of row is not same voxel
							|| (bitmap[layerY] >> subX) & 0x1 // this part of row is already in a mesh
							|| (layerZ != 0 && blockData[subX + layerY * 16 + (layerZ - 1) * 16 * 16] != 0) // this parts side is blocked off
							|| (layerZ == 0 && neighbors[Utilities::NORTH] && neighbors[Utilities::NORTH][subX + layerY * 16 + 15 * 16 * 16] != 0))
						{
							lengthX = subX - layerX;
							break;
						}

						bitmap[layerY] |= 0x1 << subX;

						// if we have reached the last voxel on the x axis t hen 
						if (subX == 15)
						{
							lengthX = subX - layerX + 1;
							break;
						}
					}

					// try to expand to the +y direction
					int lengthY = 1;
					for (int subY = layerY + 1; subY < 16; subY++)
					{
						bool completeRow = true;
						for (int subX = layerX; subX < layerX + lengthX; subX++)
						{
							// demorgans law here to allow short circuit might help
							if (blockData[subX + subY * 16 + layerZ * 16 * 16] != voxelType
								|| ((bitmap[subY] >> subX) & 0x1)
								|| (layerZ != 0 && blockData[subX + subY * 16 + (layerZ - 1) * 16 * 16] != 0)
								|| (layerZ == 0 && neighbors[Utilities::NORTH] && neighbors[Utilities::NORTH][subX + subY * 16 + 15 * 16 * 16] != 0))
							{
								completeRow = false;
								break;
							}
						}

						if (!completeRow)
							break;

						lengthY += 1;

						// else continue looping and mark whole row in bit map
						int16_t voxelMask = 0x1 << layerX;
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
	for (int layerZ = 0; layerZ < 16; layerZ++)
	{
		int16_t bitmap[16] = { 0 };
		for (int layerY = 0; layerY < 16; layerY++)
		{
			for (int layerX = 0; layerX < 16; layerX++)
			{
				if (blockData[layerX + layerY * 16 + layerZ * 16 * 16] > 0 && !((bitmap[layerY] >> layerX) & 0x1))
				{
					if (layerZ != 15 && blockData[layerX + layerY * 16 + (layerZ + 1) * 16 * 16] != 0)
						continue;

					if (layerZ == 15 && neighbors[Utilities::SOUTH] && neighbors[Utilities::SOUTH][layerX + layerY * 16 + 0 * 16 * 16] != 0)
						continue;

					int voxelType = blockData[layerX + layerY * 16 + layerZ * 16 * 16];

					bitmap[layerY] |= 0x1 << layerX;

					int lengthX = 1;
					for (int subX = layerX + 1; subX < 16; subX++)
					{
						if (blockData[subX + layerY * 16 + layerZ * 16 * 16] != voxelType
							|| (bitmap[layerY] >> subX) & 0x1
							|| (layerZ != 15 && blockData[subX + layerY * 16 + (layerZ + 1) * 16 * 16] != 0)
							|| (layerZ == 15 && neighbors[Utilities::SOUTH] && neighbors[Utilities::SOUTH][subX + layerY * 16 + 0 * 16 * 16] != 0))
						{
							lengthX = subX - layerX;
							break;
						}

						bitmap[layerY] |= 0x1 << subX;

						if (subX == 15)
						{
							lengthX = subX - layerX + 1;
							break;
						}
					}

					int lengthY = 1;
					for (int subY = layerY + 1; subY < 16; subY++)
					{
						bool completeRow = true;
						for (int subX = layerX; subX < layerX + lengthX; subX++)
						{
							if (blockData[subX + subY * 16 + layerZ * 16 * 16] != voxelType
								|| ((bitmap[subY] >> subX) & 0x1)
								|| (layerZ != 15 && blockData[subX + subY * 16 + (layerZ + 1) * 16 * 16] != 0)
								|| (layerZ == 15 && neighbors[Utilities::SOUTH] && neighbors[Utilities::SOUTH][subX + subY * 16 + 0 * 16 * 16] != 0))
							{
								completeRow = false;
								break;
							}
						}

						if (!completeRow)
							break;

						lengthY += 1;

						int16_t voxelMask = 0x1 << layerX;
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
	for (int layerY = 0; layerY < 16; layerY++)
	{
		int16_t bitmap[16] = { 0 };
		for (int layerZ = 0; layerZ < 16; layerZ++)
		{
			for (int layerX = 0; layerX < 16; layerX++)
			{
				if (blockData[layerX + layerY * 16 + layerZ * 16 * 16] > 0 && !((bitmap[layerZ] >> layerX) & 0x1))
				{
					if (layerY != 15 && blockData[layerX + (layerY + 1) * 16 + layerZ * 16 * 16] != 0)
						continue;

					if (layerY == 15 && neighbors[Utilities::UP] && neighbors[Utilities::UP][layerX + 0 * 16 + layerZ * 16 * 16] != 0)
						continue;

					int voxelType = blockData[layerX + layerY * 16 + layerZ * 16 * 16];

					bitmap[layerZ] |= 0x1 << layerX;

					int lengthX = 1;
					for (int subX = layerX + 1; subX < 16; subX++)
					{
						if (blockData[subX + layerY * 16 + layerZ * 16 * 16] != voxelType
							|| (bitmap[layerZ] >> subX) & 0x1
							|| (layerY != 15 && blockData[subX + (layerY + 1) * 16 + layerZ * 16 * 16] != 0)
							|| (layerY == 15 && neighbors[Utilities::UP] && neighbors[Utilities::UP][subX + 0 * 16 + layerZ * 16 * 16] != 0))
						{
							lengthX = subX - layerX;
							break;
						}

						bitmap[layerZ] |= 0x1 << subX;

						if (subX == 15)
						{
							lengthX = subX - layerX + 1;
							break;
						}
					}

					int lengthZ = 1;
					for (int subZ = layerZ + 1; subZ < 16; subZ++)
					{
						bool completeRow = true;
						for (int subX = layerX; subX < layerX + lengthX; subX++)
						{
							if (blockData[subX + layerY * 16 + subZ * 16 * 16] != voxelType
								|| ((bitmap[subZ] >> subX) & 0x1)
								|| (layerY != 15 && blockData[subX + (layerY + 1) * 16 + subZ * 16 * 16] != 0)
								|| (layerY == 15 && neighbors[Utilities::UP] && neighbors[Utilities::UP][subX + 0 * 16 + subZ * 16 * 16] != 0))
							{
								completeRow = false;
								break;
							}
						}

						if (!completeRow)
							break;

						lengthZ += 1;

						int16_t voxelMask = 0x1 << layerX;
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
	for (int layerY = 0; layerY < 16; layerY++)
	{
		int16_t bitmap[16] = { 0 };
		for (int layerZ = 0; layerZ < 16; layerZ++)
		{
			for (int layerX = 0; layerX < 16; layerX++)
			{
				if (blockData[layerX + layerY * 16 + layerZ * 16 * 16] > 0 && !((bitmap[layerZ] >> layerX) & 0x1))
				{
					if (layerY != 0 && blockData[layerX + (layerY - 1) * 16 + layerZ * 16 * 16] != 0)
						continue;

					if (layerY == 0 && neighbors[Utilities::DOWN] && neighbors[Utilities::DOWN][layerX + 15 * 16 + layerZ * 16 * 16] != 0)
						continue;

					int voxelType = blockData[layerX + layerY * 16 + layerZ * 16 * 16];

					bitmap[layerZ] |= 0x1 << layerX;

					int lengthX = 1;
					for (int subX = layerX + 1; subX < 16; subX++)
					{
						if (blockData[subX + layerY * 16 + layerZ * 16 * 16] != voxelType
							|| (bitmap[layerZ] >> subX) & 0x1
							|| (layerY != 0 && blockData[subX + (layerY - 1) * 16 + layerZ * 16 * 16] != 0)
							|| (layerY == 0 && neighbors[Utilities::DOWN] && neighbors[Utilities::DOWN][subX + 15 * 16 + layerZ * 16 * 16] != 0))
						{
							lengthX = subX - layerX;
							break;
						}

						bitmap[layerZ] |= 0x1 << subX;

						if (subX == 15)
						{
							lengthX = subX - layerX + 1;
							break;
						}
					}

					int lengthZ = 1;
					for (int subZ = layerZ + 1; subZ < 16; subZ++)
					{
						bool completeRow = true;
						for (int subX = layerX; subX < layerX + lengthX; subX++)
						{
							if (blockData[subX + layerY * 16 + subZ * 16 * 16] != voxelType
								|| ((bitmap[subZ] >> subX) & 0x1)
								|| (layerY != 0 && blockData[subX + (layerY - 1) * 16 + subZ * 16 * 16] != 0)
								|| (layerY == 0 && neighbors[Utilities::DOWN] && neighbors[Utilities::DOWN][subX + 15 * 16 + subZ * 16 * 16] != 0))
							{
								completeRow = false;
								break;
							}
						}

						if (!completeRow)
							break;

						lengthZ += 1;

						int16_t voxelMask = 0x1 << layerX;
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
	for (int layerX = 0; layerX < 16; layerX++)
	{
		int16_t bitmap[16] = { 0 };
		for (int layerZ = 0; layerZ < 16; layerZ++)
		{
			for (int layerY = 0; layerY < 16; layerY++)
			{
				if (blockData[layerX + layerY * 16 + layerZ * 16 * 16] > 0 && !((bitmap[layerZ] >> layerY) & 0x1))
				{
					if (layerX != 15 && blockData[(layerX + 1) + layerY * 16 + layerZ * 16 * 16] != 0)
						continue;

					if (layerX == 15 && neighbors[Utilities::EAST] && neighbors[Utilities::EAST][0 + layerY * 16 + layerZ * 16 * 16] != 0)
						continue;

					int voxelType = blockData[layerX + layerY * 16 + layerZ * 16 * 16];

					bitmap[layerZ] |= 0x1 << layerY;

					int lengthY = 1;
					for (int subY = layerY + 1; subY < 16; subY++)
					{
						if (blockData[layerX + subY * 16 + layerZ * 16 * 16] != voxelType
							|| (bitmap[layerZ] >> subY) & 0x1
							|| (layerX != 15 && blockData[(layerX + 1) + subY * 16 + layerZ * 16 * 16] != 0)
							|| (layerX == 15 && neighbors[Utilities::EAST] && neighbors[Utilities::EAST][0 + subY * 16 + layerZ * 16 * 16] != 0))
						{
							lengthY = subY - layerY;
							break;
						}

						bitmap[layerZ] |= 0x1 << subY;

						if (subY == 15)
						{
							lengthY = subY - layerY + 1;
							break;
						}
					}

					int lengthZ = 1;
					for (int subZ = layerZ + 1; subZ < 16; subZ++)
					{
						bool completeRow = true;
						for (int subY = layerY; subY < layerY + lengthY; subY++)
						{
							if (blockData[layerX + subY * 16 + subZ * 16 * 16] != voxelType
								|| ((bitmap[subZ] >> subY) & 0x1)
								|| (layerX != 15 && blockData[(layerX + 1) + subY * 16 + subZ * 16 * 16] != 0)
								|| (layerX == 15 && neighbors[Utilities::EAST] && neighbors[Utilities::EAST][0 + subY * 16 + subZ * 16 * 16] != 0))
							{
								completeRow = false;
								break;
							}
						}

						if (!completeRow)
							break;

						lengthZ += 1;

						int16_t voxelMask = 0x1 << layerY;
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
	for (int layerX = 0; layerX < 16; layerX++)
	{
		int16_t bitmap[16] = { 0 };
		for (int layerZ = 0; layerZ < 16; layerZ++)
		{
			for (int layerY = 0; layerY < 16; layerY++)
			{
				if (blockData[layerX + layerY * 16 + layerZ * 16 * 16] > 0 && !((bitmap[layerZ] >> layerY) & 0x1))
				{
					if (layerX != 0 && blockData[(layerX - 1) + layerY * 16 + layerZ * 16 * 16] != 0)
						continue;

					if (layerX == 0 && neighbors[Utilities::WEST] && neighbors[Utilities::WEST][15 + layerY * 16 + layerZ * 16 * 16] != 0)
						continue;

					int voxelType = blockData[layerX + layerY * 16 + layerZ * 16 * 16];

					bitmap[layerZ] |= 0x1 << layerY;

					int lengthY = 1;
					for (int subY = layerY + 1; subY < 16; subY++)
					{
						if (blockData[layerX + subY * 16 + layerZ * 16 * 16] != voxelType
							|| (bitmap[layerZ] >> subY) & 0x1
							|| (layerX != 0 && blockData[(layerX - 1) + subY * 16 + layerZ * 16 * 16] != 0)
							|| (layerX == 0 && neighbors[Utilities::WEST] && neighbors[Utilities::WEST][15 + subY * 16 + layerZ * 16 * 16] != 0))
						{
							lengthY = subY - layerY;
							break;
						}

						bitmap[layerZ] |= 0x1 << subY;

						if (subY == 15)
						{
							lengthY = subY - layerY + 1;
							break;
						}
					}

					int lengthZ = 1;
					for (int subZ = layerZ + 1; subZ < 16; subZ++)
					{
						bool completeRow = true;
						for (int subY = layerY; subY < layerY + lengthY; subY++)
						{
							if (blockData[layerX + subY * 16 + subZ * 16 * 16] != voxelType
								|| ((bitmap[subZ] >> subY) & 0x1)
								|| (layerX != 0 && blockData[(layerX - 1) + subY * 16 + subZ * 16 * 16] != 0)
								|| (layerX == 0 && neighbors[Utilities::WEST] && neighbors[Utilities::WEST][15 + subY * 16 + subZ * 16 * 16] != 0))
							{
								completeRow = false;
								break;
							}
						}

						if (!completeRow)
							break;

						lengthZ += 1;

						int16_t voxelMask = 0x1 << layerY;
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

	auto& buckets = chunk.m_BucketIDs;

	for (int i = 0; i < 6; i++)
	{
		Utilities::DIRECTION direction = static_cast<Utilities::DIRECTION>(i);
		buckets[direction] = m_DrawPool.AllocateBucket(static_cast<int>(vertexData[direction].size()));
		auto extraData = glm::ivec4(t_ToMesh, 0);
		m_DrawPool.FillBucket(buckets[direction], vertexData[direction], direction, extraData);
	}

	TIMER_END(1, "Meshing time: ")
}
