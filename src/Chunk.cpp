#include "Chunk.hpp"

#include <iostream>

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
				
					m_BlockData[x + 16 * y + z * 16 * 16] = 1;
			}
		}
	}
}

void Chunk::RenderChunk(VAO& t_ChunkVAO, Shader& t_Shader)
{
	if (m_MeshLength == 0)
	{
		return;
	}

	t_Shader.SetVec3("ChunkPosition", m_ChunkPosition);
	t_ChunkVAO.BindVertexBuffer(m_MeshData, 0, 0, 6 * sizeof(float));
	glDrawArrays(GL_TRIANGLES, 0, m_MeshLength);
}

void Chunk::GreedyMesh()
{
	// DEBUG DATA

	//for (auto& d : m_BlockData)
	//{
	//	d = 0;
	//}

	//m_BlockData[0] = 1;
	//m_BlockData[1] = 1;
	//m_BlockData[2] = 1;
	//m_BlockData[16] = 1;
	//m_BlockData[17] = 1;
	//m_BlockData[32] = 1;

	std::vector<float> vertices;

	// Find first valid voxel
	glm::vec3 firstValidPosition{-1};

	for (int i = 0; i < 16*16*16; i++)
	{
		if (m_BlockData[i] > 0)
		{
			int x = i % 16;
			int z = i / (16 * 16);
			int y = i - x - z;
			firstValidPosition = glm::vec3(x, y, z);
			break;
		}
	}

	if (firstValidPosition == glm::vec3{ -1 })
	{
		// Empty Chunk
		return;
	}

	// x-y bitmap, need 16^2 bits, 16 * 16bit numbers
	int16_t xyBitmap[16] = {0}; // xyBitmap[0] is x row of y = 0, xyBitmap[1] is x row of y = 1

	//TODO: add a loop here for z, then we need to do this 5 times on different planes
	// xy reverse, xz, xz reverse, yz, yz reverse
	// just do z = 0 right now
	for (int layerY = 0; layerY < 16; layerY++)
	{
		for (int layerX = 0; layerX < 16; layerX++)
		{
			// find a valid block, that hasn't been meshed yet,
			if (m_BlockData[layerX + layerY * 16] > 0 && !((xyBitmap[layerY] >> layerX) & 0x1))
			{
				// Looking at voxel at [layerX, layerY]
				int voxelType = m_BlockData[layerX + layerY * 16];
				//TODO: do optimization by moving layerX by lengthX, just an easy skip, remember that we layerX++, but maybe that should be removed?

				// mark the current voxel
				xyBitmap[layerY] |= 0x1 << layerX;

				// expand mesh to the +x direction
				int lengthX = 1; // length of side, default 1 since it's 
				for (int subX = layerX + 1; subX < 16; subX++)
				{
					// expand till we reach a voxel of different type - or air
					if (m_BlockData[subX + layerY * 16] != voxelType || ((xyBitmap[layerY] >> subX) & 0x1))
					{
						lengthX = subX - layerX;
						break;
					}

					xyBitmap[layerY] |= 0x1 << subX;

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
						if (m_BlockData[subX + subY * 16] != voxelType || ((xyBitmap[subY] >> subX) & 0x1))
						{
							completeRow = false;
							break;
						}
					}

					// if we reached a row that wasn't complete, create our mesh up to last row
					// or if we are at end of layer (y=15) 
					if (!completeRow || (completeRow && subY == 15))
					{
						if (completeRow && subY == 15)
							lengthY++;
						// end mesh at subY - 1, so we are meshing the square area...
						// [layerX, layerY] -> [layerX + lengthX - 1, layerY + lengthY - 1]
						std::cout << "[" << layerX << ", " << layerY << "] -> [" << layerX + lengthX << ", " << layerY + lengthY << "]\n";
						glm::vec2 bottomLeft { static_cast<float>(layerX), static_cast<float>(layerY) };
						glm::vec2 topLeft { static_cast<float>(layerX), static_cast<float>(layerY + lengthY ) };
						glm::vec2 bottomRight{ static_cast<float>(layerX + lengthX), static_cast<float>(layerY) };
						glm::vec2 topRight { static_cast<float>(layerX + lengthX), static_cast<float>(layerY + lengthY) };
						std::vector<float> verts = {bottomLeft.x, bottomLeft.y, 0, 0.5, 0.5, 0.5,
						bottomRight.x, bottomRight.y, 0, 0.5, 0.5, 0.5,
						topRight.x, topRight.y, 0, 0.5, 0.5, 0.5,
						bottomLeft.x, bottomLeft.y, 0, 0.5, 0.5, 0.5,
						topLeft.x, topLeft.y, 0, 0.5, 0.5, 0.5,
						topRight.x, topRight.y, 0, 0.5, 0.5, 0.5};
						vertices.insert(vertices.end(), verts.begin(), verts.end());
						break;
					}

					lengthY += 1;

					// else continue looping and mark whole row in bit map
					int16_t voxelMask = 0x1 << layerX;
					for (int i = layerX; i < layerX + lengthX; i++)
					{
						voxelMask |= 0x1 << i;
					}
					xyBitmap[subY] |= voxelMask;
				}
 			}
		}
	}

	m_MeshData.SetBufferData(vertices);
	m_MeshLength = static_cast<int>(vertices.size());

	// go top side
	for (int tz = 0; tz < 16; tz++)
	{

	}
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
					std::vector<float> newCube (18 * 6 + 6);

					const float xf = static_cast<float>(x);
					const float yf = static_cast<float>(y);
					const float zf = static_cast<float>(z);

					// check neighbor voxels if empty we need to add our side
					// East (+x)
					if (x == 15 || m_BlockData[(x + 1) + 16 * y + z * 16 * 16] == 0)
					{
						 float side[] = {
						 0.5f,  0.5f,  0.5f,
						 0.5f,  0.5f, -0.5f,
						 0.5f, -0.5f, -0.5f,
						 0.5f, -0.5f, -0.5f,
						 0.5f, -0.5f,  0.5f,
						 0.5f,  0.5f,  0.5f };

						for (int i = 0; i < 18; i+=3)
						{
							side[i] += xf;
							side[i + 1] += yf;
							side[i + 2] += zf;
						}

						newCube.insert(newCube.end(), &side[0], &side[sizeof(side) / sizeof(float)]);
					}

					// West (-x)
					if (x == 0 || m_BlockData[(x - 1) + 16 * y + z * 16 * 16] == 0)
					{
						float side[] = {
						-0.5f,  0.5f,  0.5f,
						-0.5f,  0.5f, -0.5f,
						-0.5f, -0.5f, -0.5f,
						-0.5f, -0.5f, -0.5f,
						-0.5f, -0.5f,  0.5f,
						-0.5f,  0.5f,  0.5f };

						for (int i = 0; i < 18; i += 3)
						{
							side[i] += xf;
							side[i + 1] += yf;
							side[i + 2] += zf;
						}

						newCube.insert(newCube.end(), &side[0], &side[sizeof(side) / sizeof(float)]);
					}

					// Up (+y)
					if (y == 15 || m_BlockData[x + 16 * (y + 1) + z * 16 * 16] == 0)
					{
						float side[] = {
						 -0.5f,  0.5f, -0.5f,
						 0.5f,  0.5f, -0.5f,
						 0.5f,  0.5f,  0.5f,
						 0.5f,  0.5f,  0.5f,
						-0.5f,  0.5f,  0.5f,
						-0.5f,  0.5f, -0.5f };

						for (int i = 0; i < 18; i += 3)
						{
							side[i] += xf;
							side[i + 1] += yf;
							side[i + 2] += zf;
						}
						newCube.insert(newCube.end(), &side[0], &side[sizeof(side) / sizeof(float)]);
					}

					// Down (-y)
					if (y == 0 || m_BlockData[x + 16 * (y - 1) + z * 16 * 16] == 0)
					{
						float side[] = {
						-0.5f, -0.5f, -0.5f,
						 0.5f, -0.5f, -0.5f,
						 0.5f, -0.5f,  0.5f,
						 0.5f, -0.5f,  0.5f,
						-0.5f, -0.5f,  0.5f,
						-0.5f, -0.5f, -0.5f };

						for (int i = 0; i < 18; i += 3)
						{
							side[i] += xf;
							side[i + 1] += yf;
							side[i + 2] += zf;
						}
						newCube.insert(newCube.end(), &side[0], &side[sizeof(side) / sizeof(float)]);
					}

					// South (+z)
					if (z == 15 || m_BlockData[x + 16 * y + (z + 1) * 16 * 16] == 0)
					{
						float side[] = {
						 -0.5f, -0.5f,  0.5f,
						 0.5f, -0.5f,  0.5f,
						 0.5f,  0.5f,  0.5f,
						 0.5f,  0.5f,  0.5f,
						-0.5f,  0.5f,  0.5f,
						-0.5f, -0.5f,  0.5f };

						for (int i = 0; i < 18; i += 3)
						{
							side[i] += xf;
							side[i + 1] += yf;
							side[i + 2] += zf;
						}
						newCube.insert(newCube.end(), &side[0], &side[sizeof(side) / sizeof(float)]);
					}

					// North (-z)
					if (z == 0 || m_BlockData[x + 16 * y + (z - 1) * 16 * 16] == 0)
					{
						float side[] = {
						-0.5f, -0.5f, -0.5f,
						 0.5f, -0.5f, -0.5f,
						 0.5f,  0.5f, -0.5f,
						 0.5f,  0.5f, -0.5f,
						-0.5f,  0.5f, -0.5f,
						-0.5f, -0.5f, -0.5f };

						for (int i = 0; i < 18; i += 3)
						{
							side[i] += xf;
							side[i + 1] += yf;
							side[i + 2] += zf;
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

	std::vector<float> v = {
		// positions         // colors
		 0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,   // bottom right
		-0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,   // bottom left
		 0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f    // top 
	};

	m_MeshData.SetBufferData(v);
	m_MeshLength = static_cast<int>(v.size());


}

void Chunk::SetChunkNeighbor(Utilities::DIRECTION t_Direction, Chunk* t_Neighbor)
{
	m_ChunkNeighbors[t_Direction] = t_Neighbor;
}
