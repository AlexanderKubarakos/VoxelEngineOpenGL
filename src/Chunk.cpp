#include "Chunk.hpp"

#include <iostream>

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
	t_ChunkVAO.BindVertexBuffer(m_MeshData, 0, 0, 3 * sizeof(float));
	glDrawArrays(GL_TRIANGLES, 0, m_MeshLength);
}

void Chunk::MeshChunk()
{
    std::vector<float> cube
	{
		-0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f,  0.5f, -0.5f,
		 0.5f,  0.5f, -0.5f,
		-0.5f,  0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,

		-0.5f, -0.5f,  0.5f,
		 0.5f, -0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
		-0.5f, -0.5f,  0.5f,

		-0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,

		 0.5f,  0.5f,  0.5f,
		 0.5f,  0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,

		-0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f,  0.5f,
		 0.5f, -0.5f,  0.5f,
		-0.5f, -0.5f,  0.5f,
		-0.5f, -0.5f, -0.5f,

		-0.5f,  0.5f, -0.5f,
		 0.5f,  0.5f, -0.5f,
		 0.5f,  0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f, -0.5f
    };

	std::vector<float> vertices;

	for (int x = 0; x < 16; x++)
	{
		for (int y = 0; y < 16; y++)
		{
			for (int z = 0; z < 16; z++)
			{
				// if voxel exists
				if (m_BlockData[x + 16 * y + z * 16 * 16] > 0)
				{
					std::vector<float> newCube (cube.size());

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
						-0.5f, -0.5f, -0.5f + zf,
						 0.5f, -0.5f, -0.5f + zf,
						 0.5f,  0.5f, -0.5f + zf,
						 0.5f,  0.5f, -0.5f + zf,
						-0.5f,  0.5f, -0.5f + zf,
						-0.5f, -0.5f, -0.5f + zf };

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
}

void Chunk::SetChunkNeighbor(Utilities::DIRECTION t_Direction, Chunk* t_Neighbor)
{
	m_ChunkNeighbors[t_Direction] = t_Neighbor;
}
