#pragma once
#include "DrawPool.hpp"
#include "glm/vec3.hpp"

class Chunk
{
public:
	static constexpr int CHUNK_DIMESION = 32;
	static constexpr int CHUNK_DATA_SIZE_BTYES = sizeof(int8_t) * CHUNK_DIMESION * CHUNK_DIMESION * CHUNK_DIMESION;
	Chunk(glm::ivec3 t_ChunkPosition);
	~Chunk();

	Chunk(const Chunk& t_Other) = delete;
	Chunk(Chunk&& t_Other) noexcept
		: m_BlockData(t_Other.m_BlockData),
		  m_ChunkPosition(std::move(t_Other.m_ChunkPosition)),
		  m_BucketIDs(std::move(t_Other.m_BucketIDs)),
		  allAir(t_Other.allAir)
	{
		t_Other.m_BlockData = nullptr;
	}

	Chunk& operator=(const Chunk& t_Other) = delete;
	Chunk& operator=(Chunk&& t_Other) noexcept
	{
		if (this == &t_Other)
			return *this;
		m_BlockData = t_Other.m_BlockData;
		t_Other.m_BlockData = nullptr;
		m_ChunkPosition = std::move(t_Other.m_ChunkPosition);
		m_BucketIDs = std::move(t_Other.m_BucketIDs);
		allAir = t_Other.allAir;
		return *this;
	}

	void setVoxel(int x, int y, int z, int8_t type) { m_BlockData[x + y * 32 + z * 32 * 32] = type; }
	int8_t getVoxel(int x, int y, int z) const { return m_BlockData[x + y * 32 + z * 32 * 32]; };
	void setVoxel(const glm::ivec3& pos, int8_t type) { m_BlockData[pos.x + pos.y * 32 + pos.z * 32 * 32] = type; }
	int8_t getVoxel(const glm::ivec3& pos) const { return m_BlockData[pos.x + pos.y * 32 + pos.z * 32 * 32]; };

	// Position of chunk relative to other chunk
	glm::ivec3 m_ChunkPosition;
	// Data for voxels, 16^3
	int8_t* m_BlockData;
	// Is chunk all air?
	bool allAir;
	// Bucket ID of supplied DrawPool
	std::array<DrawPool::BucketID, 6> m_BucketIDs;
};
