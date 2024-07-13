#include "Chunk.hpp"

#include <chrono>
#include <iostream>

#include "imgui.h"

struct Vertex
{
	float pos[3];
	float color[4];

	Vertex(glm::vec3& t_pos, glm::vec3& t_color)
	{
		pos[0] = t_pos.x;
		pos[1] = t_pos.y;
		pos[2] = t_pos.z;

		color[0] = t_color.x;
		color[1] = t_color.y;
		color[2] = t_color.z;
		color[3] = 1.0f;
	}
};

Chunk::Chunk(glm::vec3 t_ChunkPosition) : m_ChunkPosition(t_ChunkPosition)
{
	for (int x = 0; x < 16; x++)
	{
		for (int y = 0; y < 16; y++)
		{
			for (int z = 0; z < 16; z++)
			{
				if (rand() % 100 > 80)
					m_BlockData[x + 16 * y + z * 16 * 16] = 1;
				if (y < 8)
					m_BlockData[x + 16 * y + z * 16 * 16] = 1;
			}
		}
	}

	//m_BlockData[8 + 15 * 16 + 4 * 16 * 16] = 1;
	//m_BlockData[8 + 15 * 16 + 5 * 16 * 16] = 1;
	//m_BlockData[9 + 14 * 16 + 4 * 16 * 16] = 1;
}

void Chunk::RenderChunk(VAO& t_ChunkVAO, Shader& t_Shader)
{
	if (m_MeshLength == 0)
	{
		return;
	}

	ImGui::Text("Chunk at [x:%i y:%i z:%i] Tris count: %i", (int)m_ChunkPosition.x, (int)m_ChunkPosition.y, (int)m_ChunkPosition.z, m_MeshLength/6);

	t_Shader.SetVec3("ChunkPosition", m_ChunkPosition);
	t_ChunkVAO.BindVertexBuffer(m_MeshData, 0, 0, 6 * sizeof(float));
	glDrawArrays(GL_TRIANGLES, 0, m_MeshLength);
}

auto sideAdd = [](std::vector<float>& verts, int bottomX, int bottomY, int lengthX, int lengthY, int z)
	{
		glm::vec2 bottomLeft{ static_cast<float>(bottomX), static_cast<float>(bottomY) };
		glm::vec2 topLeft{ static_cast<float>(bottomX), static_cast<float>(bottomY + lengthY) };
		glm::vec2 bottomRight{ static_cast<float>(bottomX + lengthX), static_cast<float>(bottomY) };
		glm::vec2 topRight{ static_cast<float>(bottomX + lengthX), static_cast<float>(bottomY + lengthY) };
		float fz = static_cast<float>(z);
		float arr[] = { bottomLeft.x, bottomLeft.y, fz, 0.7f, 0.3f, 0.1f,
		bottomRight.x, bottomRight.y, fz, 0.7f, 0.3f, 0.1f,
		topRight.x, topRight.y, fz, 0.7f, 0.3f, 0.1f,
		bottomLeft.x, bottomLeft.y, fz, 0.7f, 0.3f, 0.1f,
		topLeft.x, topLeft.y, fz, 0.7f, 0.3f, 0.1f,
		topRight.x, topRight.y, fz, 0.7f, 0.3f, 0.1f };
		verts.insert(verts.end(), &arr[0], &arr[sizeof(arr) / sizeof(float)]);
	};

auto sideAddXZ = [](std::vector<float>& verts, int bottomX, int bottomZ, int lengthX, int lengthZ, int y)
	{
		glm::vec2 bottomLeft{ static_cast<float>(bottomX), static_cast<float>(bottomZ) };
		glm::vec2 topLeft{ static_cast<float>(bottomX), static_cast<float>(bottomZ + lengthZ) };
		glm::vec2 bottomRight{ static_cast<float>(bottomX + lengthX), static_cast<float>(bottomZ) };
		glm::vec2 topRight{ static_cast<float>(bottomX + lengthX), static_cast<float>(bottomZ + lengthZ) };
		float fy = static_cast<float>(y);
		float arr[] = { bottomLeft.x, fy, bottomLeft.y, 0.7f, 0.3f, 0.1f,
		bottomRight.x, fy, bottomRight.y, 0.7f, 0.3f, 0.1f,
		topRight.x, fy, topRight.y, 0.7f, 0.3f, 0.1f,
		bottomLeft.x, fy, bottomLeft.y, 0.7f, 0.3f, 0.1f,
		topLeft.x, fy, topLeft.y, 0.7f, 0.3f, 0.1f,
		topRight.x, fy, topRight.y, 0.7f, 0.3f, 0.1f };
		verts.insert(verts.end(), &arr[0], &arr[sizeof(arr) / sizeof(float)]);
	};

auto sideAddYZ = [](std::vector<float>& verts, int bottomY, int bottomZ, int lengthY, int lengthZ, int x)
	{
		glm::vec2 bottomLeft{ static_cast<float>(bottomY), static_cast<float>(bottomZ) };
		glm::vec2 topLeft{ static_cast<float>(bottomY), static_cast<float>(bottomZ + lengthZ) };
		glm::vec2 bottomRight{ static_cast<float>(bottomY + lengthY), static_cast<float>(bottomZ) };
		glm::vec2 topRight{ static_cast<float>(bottomY + lengthY), static_cast<float>(bottomZ + lengthZ) };
		float fx = static_cast<float>(x);
		float arr[] = { fx, bottomLeft.x, bottomLeft.y, 0.7f, 0.3f, 0.1f,
		fx, bottomRight.x, bottomRight.y, 0.7f, 0.3f, 0.1f,
		fx, topRight.x, topRight.y, 0.7f, 0.3f, 0.1f,
		fx, bottomLeft.x, bottomLeft.y, 0.7f, 0.3f, 0.1f,
		fx, topLeft.x, topLeft.y, 0.7f, 0.3f, 0.1f,
		fx, topRight.x, topRight.y, 0.7f, 0.3f, 0.1f };
		verts.insert(verts.end(), &arr[0], &arr[sizeof(arr) / sizeof(float)]);
	};

void Chunk::GreedyMesh()
{
	std::vector<float> vertices;

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
							
					sideAdd(vertices, layerX, layerY, lengthX, lengthY, layerZ);
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

					sideAdd(vertices, layerX, layerY, lengthX, lengthY, layerZ + 1);
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

					sideAddXZ(vertices, layerX, layerZ, lengthX, lengthZ, layerY + 1);
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

					sideAddXZ(vertices, layerX, layerZ, lengthX, lengthZ, layerY);
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

					sideAddYZ(vertices, layerY, layerZ, lengthY, lengthZ, layerX + 1);
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

					sideAddYZ(vertices, layerY, layerZ, lengthY, lengthZ, layerX);
				}
			}
		}
	}

	m_MeshData.SetBufferData(vertices);
	m_MeshLength = static_cast<int>(vertices.size());
}

void Chunk::MeshChunk()
{
	std::vector<float> vertices;

	for (int z = 0; z < 16; z++)
	{
		for (int y = 0; y < 16; y++)
		{
			for (int x = 0; x < 16; x++)
			{
				// if voxel exists
				if (m_BlockData[x + 16 * y + z * 16 * 16] > 0)
				{
					std::vector<float> newCube;
					newCube.reserve(256);

					const float xf = static_cast<float>(x);
					const float yf = static_cast<float>(y);
					const float zf = static_cast<float>(z);

					// check neighbor voxels if empty we need to add our side
					// East (+x)
					if (x == 15 || m_BlockData[(x + 1) + 16 * y + z * 16 * 16] == 0)
					{
						 float side[] = {
						 0.5f,  0.5f,  0.5f, 0.7f, 0.3f, 0.1f,
						 0.5f,  0.5f, -0.5f, 0.7f, 0.3f, 0.1f,
						 0.5f, -0.5f, -0.5f, 0.7f, 0.3f, 0.1f,
						 0.5f, -0.5f, -0.5f, 0.7f, 0.3f, 0.1f,
						 0.5f, -0.5f,  0.5f, 0.7f, 0.3f, 0.1f,
						 0.5f,  0.5f,  0.5f, 0.7f, 0.3f, 0.1f };

						for (int i = 0; i < 6; i++)
						{
							side[i * 6] += xf;
							side[i * 6 + 1] += yf;
							side[i * 6 + 2] += zf;
						}

						newCube.insert(newCube.end(), &side[0], &side[sizeof(side) / sizeof(float)]);
					}

					// West (-x)
					if (x == 0 || m_BlockData[(x - 1) + 16 * y + z * 16 * 16] == 0)
					{
						float side[] = {
						-0.5f,  0.5f,  0.5f, 0.7f, 0.3f, 0.1f,
						-0.5f,  0.5f, -0.5f, 0.7f, 0.3f, 0.1f,
						-0.5f, -0.5f, -0.5f, 0.7f, 0.3f, 0.1f,
						-0.5f, -0.5f, -0.5f, 0.7f, 0.3f, 0.1f,
						-0.5f, -0.5f,  0.5f, 0.7f, 0.3f, 0.1f,
						-0.5f,  0.5f,  0.5f, 0.7f, 0.3f, 0.1f };

						for (int i = 0; i < 6; i++)
						{
							side[i * 6] += xf;
							side[i * 6 + 1] += yf;
							side[i * 6 + 2] += zf;
						}

						newCube.insert(newCube.end(), &side[0], &side[sizeof(side) / sizeof(float)]);
					}

					// Up (+y)
					if (y == 15 || m_BlockData[x + 16 * (y + 1) + z * 16 * 16] == 0)
					{
						float side[] = {
						 -0.5f,  0.5f, -0.5f, 0.7f, 0.3f, 0.1f,
						 0.5f,  0.5f, -0.5f, 0.7f, 0.3f, 0.1f,
						 0.5f,  0.5f,  0.5f, 0.7f, 0.3f, 0.1f,
						 0.5f,  0.5f,  0.5f, 0.7f, 0.3f, 0.1f,
						-0.5f,  0.5f,  0.5f, 0.7f, 0.3f, 0.1f,
						-0.5f,  0.5f, -0.5f, 0.7f, 0.3f, 0.1f };

						for (int i = 0; i < 6; i++)
						{
							side[i * 6] += xf;
							side[i * 6 + 1] += yf;
							side[i * 6 + 2] += zf;
						}
						newCube.insert(newCube.end(), &side[0], &side[sizeof(side) / sizeof(float)]);
					}

					// Down (-y)
					if (y == 0 || m_BlockData[x + 16 * (y - 1) + z * 16 * 16] == 0)
					{
						float side[] = {
						-0.5f, -0.5f, -0.5f, 0.7f, 0.3f, 0.1f,
						 0.5f, -0.5f, -0.5f, 0.7f, 0.3f, 0.1f,
						 0.5f, -0.5f,  0.5f, 0.7f, 0.3f, 0.1f,
						 0.5f, -0.5f,  0.5f, 0.7f, 0.3f, 0.1f,
						-0.5f, -0.5f,  0.5f, 0.7f, 0.3f, 0.1f,
						-0.5f, -0.5f, -0.5f, 0.7f, 0.3f, 0.1f };

						for (int i = 0; i < 6; i++)
						{
							side[i * 6] += xf;
							side[i * 6 + 1] += yf;
							side[i * 6 + 2] += zf;
						}
						newCube.insert(newCube.end(), &side[0], &side[sizeof(side) / sizeof(float)]);
					}

					// South (+z)
					if (z == 15 || m_BlockData[x + 16 * y + (z + 1) * 16 * 16] == 0)
					{
						float side[] = {
						 -0.5f, -0.5f,  0.5f, 0.7f, 0.3f, 0.1f,
						 0.5f, -0.5f,  0.5f, 0.7f, 0.3f, 0.1f,
						 0.5f,  0.5f,  0.5f, 0.7f, 0.3f, 0.1f,
						 0.5f,  0.5f,  0.5f, 0.7f, 0.3f, 0.1f,
						-0.5f,  0.5f,  0.5f, 0.7f, 0.3f, 0.1f,
						-0.5f, -0.5f,  0.5f, 0.7f, 0.3f, 0.1f};

						for (int i = 0; i < 6; i++)
						{
							side[i * 6] += xf;
							side[i * 6 + 1] += yf;
							side[i * 6 + 2] += zf;
						}
						newCube.insert(newCube.end(), &side[0], &side[sizeof(side) / sizeof(float)]);
					}

					// North (-z)
					if (z == 0 || m_BlockData[x + 16 * y + (z - 1) * 16 * 16] == 0)
					{
						float side[] = {
						-0.5f, -0.5f, -0.5f, 0.7f, 0.3f, 0.1f,
						 0.5f, -0.5f, -0.5f, 0.7f, 0.3f, 0.1f,
						 0.5f,  0.5f, -0.5f, 0.7f, 0.3f, 0.1f,
						 0.5f,  0.5f, -0.5f, 0.7f, 0.3f, 0.1f,
						-0.5f,  0.5f, -0.5f, 0.7f, 0.3f, 0.1f,
						-0.5f, -0.5f, -0.5f, 0.7f, 0.3f, 0.1f };

						for (int i = 0; i < 6; i++)
						{
							side[i * 6] += xf;
							side[i * 6 + 1] += yf;
							side[i * 6 + 2] += zf;
						}
						newCube.insert(newCube.end(), &side[0], &side[sizeof(side) / sizeof(float)]);
					}

					vertices.insert(vertices.end(), newCube.begin(), newCube.end());
				}
			}
		}
	}

	m_MeshData.SetBufferData(vertices);
	m_MeshLength = static_cast<int>(vertices.size());
}

void Chunk::SetChunkNeighbor(Utilities::DIRECTION t_Direction, Chunk* t_Neighbor)
{
	m_ChunkNeighbors[t_Direction] = t_Neighbor;
}
