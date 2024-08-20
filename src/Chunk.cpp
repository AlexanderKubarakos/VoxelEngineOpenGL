#include "Chunk.hpp"
#define FASTNOISE_STATIC_LIB
#include <FastNoise/FastNoise.h>

Chunk::Chunk(glm::ivec3 t_ChunkPosition) : m_ChunkPosition{t_ChunkPosition}, m_BlockData{ new int8_t[4096]() }, m_BucketIDs{nullptr}
{
	static FastNoise::SmartNode<> fnGenerator = FastNoise::NewFromEncodedNodeTree("EQACAAAAAAAgQBAAAAAAQBkAEwDD9Sg/DQAEAAAAAAAgQAkAAGZmJj8AAAAAPwEEAAAAAAAAAEBAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAM3MTD4AMzMzPwAAAAA/");
	std::vector<float> noiseOutput(16 * 16 * 16);
	fnGenerator->GenUniformGrid3D(noiseOutput.data(), t_ChunkPosition.x * 16, t_ChunkPosition.y * 16, t_ChunkPosition.z * 16, 16, 16, 16, 0.002f, 1337);

	for (int z = 0; z < 16; z++)
	{
		for (int y = 0; y < 16; y++)
		{
			for (int x = 0; x < 16; x++)
			{
				if (noiseOutput[x + 16 * y + z * 16 * 16] < 0)
					m_BlockData[x + 16 * y + z * 16 * 16] = 1;
			}
		}
	}
}

Chunk::~Chunk()
{
	delete[] m_BlockData;
}
