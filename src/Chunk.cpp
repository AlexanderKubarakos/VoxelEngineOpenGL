#include "Chunk.hpp"

Chunk::Chunk(glm::vec3 t_ChunkPosition) : m_ChunkPosition(t_ChunkPosition)
{
	for (int x = 0; x < 16; x++)
	{
		for (int y = 0; y < 16; y++)
		{
			for (int z = 0; z < 16; z++)
			{
				if (rand() % 10 < 1)
				{
					m_BlockData[x + 16 * y + z * 16 * 16] = 1;
				}
			}
		}
	}
}

void Chunk::RenderChunk(VAO& t_ChunkVAO)
{
	if (m_MeshLength == 0)
	{
		return;
	}

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
				if(m_BlockData[x + 16 * y + z * 16 * 16] == 1)
				{
					// make this a stack allocation
					std::vector<float> newCube = cube;
					for (size_t i = 0; i < newCube.size(); i+=3)
					{
						newCube[i] += x + m_ChunkPosition.x * 16;
						newCube[i+1] += y + m_ChunkPosition.y * 16;
						newCube[i+2] += z + m_ChunkPosition.z * 16;
					}
					vertices.insert(vertices.cend(), newCube.begin(), newCube.end());
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
