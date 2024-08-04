#include "Chunk.hpp"

#include <chrono>
#include <iostream>

Chunk::Chunk(glm::ivec3 t_ChunkPosition, DrawPool& t_DrawPool) : m_ChunkPosition(t_ChunkPosition), m_DrawPool(t_DrawPool), m_BucketIDs {nullptr}
{
	for (int x = 0; x < 16; x++)
	{
		for (int y = 0; y < 16; y++)
		{
			for (int z = 0; z < 16; z++)
			{
				if (rand() % 100 > 10)
					m_BlockData[x + 16 * y + z * 16 * 16] = 1;
				//if (y < 13)
					m_BlockData[x + 16 * y + z * 16 * 16] = 1;
			}
		}
	}
}

void Chunk::MeshChunk()
{
	GreedyMesh();
}

void Chunk::GreedyMesh()
{
	for (auto id : m_BucketIDs)
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
				// find a valid block, that hasn't been meshed yet,
				// TODO: add check here for if voxels face should be shown, so we skip non visible
				if (m_BlockData[layerX + layerY * 16 + layerZ * 16 * 16] > 0 && !((bitmap[layerY] >> layerX) & 0x1))
				{
					if (layerZ != 0 && m_BlockData[layerX + layerY * 16 + (layerZ - 1) * 16 * 16] != 0)
						continue;
					// Looking at voxel at [layerX, layerY]
					int voxelType = m_BlockData[layerX + layerY * 16 + layerZ * 16 * 16];
					//TODO: do optimization by moving layerX by lengthX, just an easy skip, remember that we layerX++, but maybe that should be removed?

					// mark the current voxel
					bitmap[layerY] |= 0x1 << layerX;

					// expand mesh to the +x direction
					int lengthX = 1; // length of side, default 1 since it's minimum one voxel
					for (int subX = layerX + 1; subX < 16; subX++)
					{
						// TODO
						// if (subY != 15 && subX != 15 && m_BlockData[(subX + 1) + (layerY + 1) * 16 + layerZ * 16 * 16]
						// expand till we reach a voxel of different type - or air
						// these are our stops conditions, if next voxel isn't correct type, if voxel has already been explored, and if both pass
						// then check if next side should be visible

						if (m_BlockData[subX + layerY * 16 + layerZ * 16 * 16] != voxelType // this part of row is not same voxel
							|| (bitmap[layerY] >> subX) & 0x1 // this part of row is already in a mesh
							|| (layerZ != 0 && m_BlockData[subX + layerY * 16 + (layerZ - 1) * 16 * 16] != 0)) // this parts side is blocked off
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
							if (m_BlockData[subX + subY * 16 + layerZ * 16 * 16] != voxelType
								|| ((bitmap[subY] >> subX) & 0x1)
								|| (layerZ != 0 && m_BlockData[subX + subY * 16 + (layerZ - 1) * 16 * 16] != 0))
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
					Vertex bottomLeft{ layerX, layerY, layerZ};
					Vertex topLeft{ layerX, layerY + lengthY, layerZ};
					Vertex bottomRight{ layerX + lengthX, layerY, layerZ};
					Vertex topRight{ layerX + lengthX, layerY + lengthY, layerZ};

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
				if (m_BlockData[layerX + layerY * 16 + layerZ * 16 * 16] > 0 && !((bitmap[layerY] >> layerX) & 0x1))
				{
					if (layerZ != 15 && m_BlockData[layerX + layerY * 16 + (layerZ + 1) * 16 * 16] != 0)
						continue;
					
					int voxelType = m_BlockData[layerX + layerY * 16 + layerZ * 16 * 16];
					
					bitmap[layerY] |= 0x1 << layerX;

					int lengthX = 1; 
					for (int subX = layerX + 1; subX < 16; subX++)
					{
						if (m_BlockData[subX + layerY * 16 + layerZ * 16 * 16] != voxelType
							|| (bitmap[layerY] >> subX) & 0x1
							|| (layerZ != 15 && m_BlockData[subX + layerY * 16 + (layerZ + 1) * 16 * 16] != 0))
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
							if (m_BlockData[subX + subY * 16 + layerZ * 16 * 16] != voxelType
								|| ((bitmap[subY] >> subX) & 0x1)
								|| (layerZ != 15 && m_BlockData[subX + subY * 16 + (layerZ + 1) * 16 * 16] != 0))
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
					Vertex bottomLeft{ layerX, layerY, tempZ};
					Vertex topLeft{ layerX, layerY + lengthY, tempZ};
					Vertex bottomRight{ layerX + lengthX, layerY, tempZ};
					Vertex topRight{ layerX + lengthX, layerY + lengthY, tempZ};

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
		if (layerY == 1)
			layerY = 1;
		int16_t bitmap[16] = { 0 };
		for (int layerZ = 0; layerZ < 16; layerZ++)
		{
			for (int layerX = 0; layerX < 16; layerX++)
			{
				if (m_BlockData[layerX + layerY * 16 + layerZ * 16 * 16] > 0 && !((bitmap[layerZ] >> layerX) & 0x1))
				{
					if (layerY != 15 && m_BlockData[layerX + (layerY + 1) * 16 + layerZ * 16 * 16] != 0)
						continue;

					int voxelType = m_BlockData[layerX + layerY * 16 + layerZ * 16 * 16];

					bitmap[layerZ] |= 0x1 << layerX;

					int lengthX = 1;
					for (int subX = layerX + 1; subX < 16; subX++)
					{
						if (m_BlockData[subX + layerY * 16 + layerZ * 16 * 16] != voxelType
							|| (bitmap[layerZ] >> subX) & 0x1
							|| (layerY != 15 && m_BlockData[subX + (layerY + 1) * 16 + layerZ * 16 * 16] != 0))
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
							if (m_BlockData[subX + layerY * 16 + subZ * 16 * 16] != voxelType
								|| ((bitmap[subZ] >> subX) & 0x1)
								|| (layerY != 15 && m_BlockData[subX + (layerY + 1) * 16 + subZ * 16 * 16] != 0))
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
					Vertex bottomLeft{ layerX, tempY, layerZ};
					Vertex topLeft{ layerX, tempY, layerZ + lengthZ};
					Vertex bottomRight{ layerX + lengthX, tempY, layerZ};
					Vertex topRight{ layerX + lengthX, tempY, layerZ + lengthZ};

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
				if (m_BlockData[layerX + layerY * 16 + layerZ * 16 * 16] > 0 && !((bitmap[layerZ] >> layerX) & 0x1))
				{
					if (layerY != 0 && m_BlockData[layerX + (layerY - 1) * 16 + layerZ * 16 * 16] != 0)
						continue;

					int voxelType = m_BlockData[layerX + layerY * 16 + layerZ * 16 * 16];

					bitmap[layerZ] |= 0x1 << layerX;

					int lengthX = 1;
					for (int subX = layerX + 1; subX < 16; subX++)
					{
						if (m_BlockData[subX + layerY * 16 + layerZ * 16 * 16] != voxelType
							|| (bitmap[layerZ] >> subX) & 0x1
							|| (layerY != 0 && m_BlockData[subX + (layerY - 1) * 16 + layerZ * 16 * 16] != 0))
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
							if (m_BlockData[subX + layerY * 16 + subZ * 16 * 16] != voxelType
								|| ((bitmap[subZ] >> subX) & 0x1)
								|| (layerY != 0 && m_BlockData[subX + (layerY - 1) * 16 + subZ * 16 * 16] != 0))
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
					Vertex bottomLeft{ layerX, layerY, layerZ};
					Vertex topLeft{ layerX, layerY, layerZ + lengthZ};
					Vertex bottomRight{ layerX + lengthX, layerY, layerZ};
					Vertex topRight{ layerX + lengthX, layerY, layerZ + lengthZ};

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
				if (m_BlockData[layerX + layerY * 16 + layerZ * 16 * 16] > 0 && !((bitmap[layerZ] >> layerY) & 0x1))
				{
					if (layerX != 15 && m_BlockData[(layerX + 1) + layerY * 16 + layerZ * 16 * 16] != 0)
						continue;

					int voxelType = m_BlockData[layerX + layerY * 16 + layerZ * 16 * 16];

					bitmap[layerZ] |= 0x1 << layerY;

					int lengthY = 1;
					for (int subY = layerY + 1; subY < 16; subY++)
					{
						if (m_BlockData[layerX + subY * 16 + layerZ * 16 * 16] != voxelType
							|| (bitmap[layerZ] >> subY) & 0x1
							|| (layerX != 15 && m_BlockData[(layerX + 1) + subY * 16 + layerZ * 16 * 16] != 0))
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
							if (m_BlockData[layerX + subY * 16 + subZ * 16 * 16] != voxelType
								|| ((bitmap[subZ] >> subY) & 0x1)
								|| (layerX != 15 && m_BlockData[(layerX + 1) + subY * 16 + subZ * 16 * 16] != 0))
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
					Vertex bottomLeft{ tempX, layerY, layerZ};
					Vertex topLeft{ tempX, layerY, layerZ + lengthZ};
					Vertex bottomRight{ tempX, layerY + lengthY, layerZ};
					Vertex topRight{ tempX, layerY + lengthY, layerZ + lengthZ};

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
				if (m_BlockData[layerX + layerY * 16 + layerZ * 16 * 16] > 0 && !((bitmap[layerZ] >> layerY) & 0x1))
				{
					if (layerX != 0 && m_BlockData[(layerX - 1) + layerY * 16 + layerZ * 16 * 16] != 0)
						continue;

					int voxelType = m_BlockData[layerX + layerY * 16 + layerZ * 16 * 16];

					bitmap[layerZ] |= 0x1 << layerY;

					int lengthY = 1;
					for (int subY = layerY + 1; subY < 16; subY++)
					{
						if (m_BlockData[layerX + subY * 16 + layerZ * 16 * 16] != voxelType
							|| (bitmap[layerZ] >> subY) & 0x1
							|| (layerX != 0 && m_BlockData[(layerX - 1) + subY * 16 + layerZ * 16 * 16] != 0))
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
							if (m_BlockData[layerX + subY * 16 + subZ * 16 * 16] != voxelType
								|| ((bitmap[subZ] >> subY) & 0x1)
								|| (layerX != 0 && m_BlockData[(layerX - 1) + subY * 16 + subZ * 16 * 16] != 0))
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
					Vertex bottomLeft{ layerX, layerY, layerZ};
					Vertex topLeft{ layerX, layerY, layerZ + lengthZ};
					Vertex bottomRight{ layerX, layerY + lengthY, layerZ};
					Vertex topRight{ layerX, layerY + lengthY, layerZ + lengthZ};

					vertexData[Utilities::DIRECTION::WEST].push_back(bottomLeft);
					vertexData[Utilities::DIRECTION::WEST].push_back(topLeft);
					vertexData[Utilities::DIRECTION::WEST].push_back(bottomRight);
					vertexData[Utilities::DIRECTION::WEST].push_back(topRight);
				}
			}
		}
	}

	for (int i = 0; i < 6; i++)
	{
		Utilities::DIRECTION direction = static_cast<Utilities::DIRECTION>(i);
		m_BucketIDs[direction] = m_DrawPool.AllocateBucket(static_cast<int>(vertexData[direction].size()));
		auto extraData = glm::ivec4(m_ChunkPosition, 0);
		m_DrawPool.FillBucket(m_BucketIDs[direction], vertexData[direction], direction, extraData);
	}
}

//void Chunk::SetChunkNeighbor(Utilities::DIRECTION t_Direction, Chunk* t_Neighbor)
//{
//	m_ChunkNeighbors[t_Direction] = t_Neighbor;
//}
