#include "Chunk.hpp"

#include "FastNoiseLite.h"

Chunk::Chunk(glm::ivec3 t_ChunkPosition) : m_ChunkPosition{t_ChunkPosition}, m_BlockData{ std::make_unique<int8_t[]>(4096) }, m_BucketIDs{ nullptr }
{
	FastNoiseLite f{ 3333 };
	
	for (int x = 0; x < 16; x++)
	{
		for (int y = 0; y < 16; y++)
		{
			for (int z = 0; z < 16; z++)
			{
				if (f.GetNoise(static_cast<float>(x + t_ChunkPosition.x * 16), static_cast<float>(y + t_ChunkPosition.y * 16), static_cast<float>(z + t_ChunkPosition.z * 16)) > -0.25)
					m_BlockData[x + 16 * y + z * 16 * 16] = 1;
			}
		}
	}
}

Chunk::~Chunk() = default;
