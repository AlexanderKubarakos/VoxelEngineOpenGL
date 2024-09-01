#include "Chunk.hpp"
#define FASTNOISE_STATIC_LIB
#include <FastNoise/FastNoise.h>

Chunk::Chunk(glm::ivec3 t_ChunkPosition) : m_ChunkPosition{t_ChunkPosition}, m_BlockData{ new int8_t[32 * 32 * 32]() }, m_BucketIDs{nullptr}
{
	static FastNoise::SmartNode<> fnGenerator = FastNoise::NewFromEncodedNodeTree("DQAFAAAAAAAAQAgAAAAAAD8AAAAAAA==");
	//std::vector<float> noiseOutput(32 * 32 * 32);
	//fnGenerator->GenUniformGrid3D(noiseOutput.data(), t_ChunkPosition.x * 32, t_ChunkPosition.y * 32, t_ChunkPosition.z * 32, 32, 32, 32, 0.002f, 1337);

	for (int z = 0; z < 32; z++)
	{
		for (int y = 0; y < 32; y++)
		{
			for (int x = 0; x < 32; x++)
			{
				m_BlockData[x + 32 * y + z * 32 * 32] = 0;
				if (x % 2 || y % 2 || z % 2)
					m_BlockData[x + 32 * y + z * 32 * 32] = 1;
				//if (noiseOutput[x + 32 * y + z * 32 * 32] < 0)
				//	m_BlockData[x + 32 * y + z * 32 * 32] = 1;
			}
		}
	}
}

Chunk::~Chunk()
{
	delete[] m_BlockData;
}
