#include "Chunk.hpp"
#define FASTNOISE_STATIC_LIB
#include <FastNoise/FastNoise.h>

Chunk::Chunk(glm::ivec3 t_ChunkPosition) : m_ChunkPosition{t_ChunkPosition}, m_BlockData{ new int8_t[32 * 32 * 32]() }, m_BucketIDs{nullptr}
{
	ZoneScoped;
	static FastNoise::SmartNode<> fnGenerator = FastNoise::NewFromEncodedNodeTree("CAA=");
	static std::vector<float> noiseOutput(32 * 32 );
	fnGenerator->GenUniformGrid2D(noiseOutput.data(), t_ChunkPosition.x * 32, t_ChunkPosition.z * 32, 32, 32, 0.002f, 1337);
	allAir = true;
	for (int z = 0; z < 32; z++)
	{
		for (int y = 0; y < 32; y++)
		{
			for (int x = 0; x < 32; x++)
			{
				// this can be made a lot better
				if (noiseOutput[x + z * 32] * 32.0f > static_cast<float>(y + t_ChunkPosition.y * 32))
				{
					m_BlockData[x + 32 * y + z * 32 * 32] = 1;
					allAir = false;
				}
			}
		}
	}
}

Chunk::~Chunk()
{
	ZoneScoped;
	delete[] m_BlockData;
}
