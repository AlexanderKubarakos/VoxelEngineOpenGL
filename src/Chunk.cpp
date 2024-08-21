#include "Chunk.hpp"
#define FASTNOISE_STATIC_LIB
#include <FastNoise/FastNoise.h>

Chunk::Chunk(glm::ivec3 t_ChunkPosition) : m_ChunkPosition{t_ChunkPosition}, m_BlockData{ new int8_t[32 * 32 * 32]() }, m_BucketIDs{nullptr}
{
	static FastNoise::SmartNode<> fnGenerator = FastNoise::NewFromEncodedNodeTree("EwCamZk+GgABEQACAAAAAADgQBAAAACIQR8AFgABAAAACwADAAAAAgAAAAMAAAAEAAAAAAAAAD8BFAD//wAAAAAAAD8AAAAAPwAAAAA/AAAAAD8BFwAAAIC/AACAPz0KF0BSuB5AEwAAAKBABgAAj8J1PACamZk+AAAAAAAA4XoUPw==");
	std::vector<float> noiseOutput(32 * 32 * 32);
	fnGenerator->GenUniformGrid3D(noiseOutput.data(), t_ChunkPosition.x * 32, t_ChunkPosition.y * 32, t_ChunkPosition.z * 32, 32, 32, 32, 0.002f, 1337);

	for (int z = 0; z < 32; z++)
	{
		for (int y = 0; y < 32; y++)
		{
			for (int x = 0; x < 32; x++)
			{
				if (noiseOutput[x + 32 * y + z * 32 * 32] < 0)
					m_BlockData[x + 32 * y + z * 32 * 32] = 1;
			}
		}
	}

	LOG_PRINT("Finished chunk loading")
}

Chunk::~Chunk()
{
	delete[] m_BlockData;
}
