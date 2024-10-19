#pragma once
#include "DrawPool.hpp"
#include "glm/vec3.hpp"

class Chunk
{
public:
	Chunk(glm::ivec3 t_ChunkPosition);
	~Chunk();

	Chunk(const Chunk& t_Other) = delete;
	Chunk(Chunk&& t_Other) noexcept
		: m_BlockData(t_Other.m_BlockData),
		  m_ChunkPosition(std::move(t_Other.m_ChunkPosition)),
		  m_BucketIDs(std::move(t_Other.m_BucketIDs))
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
		return *this;
	}

	// Position of chunk relative to other chunk
	glm::ivec3 m_ChunkPosition;
	// Data for voxels, 16^3
	int8_t* m_BlockData;
	bool allAir;
	// Bucket ID of supplied DrawPool
	std::array<DrawPool::BucketID, 6> m_BucketIDs;
};
