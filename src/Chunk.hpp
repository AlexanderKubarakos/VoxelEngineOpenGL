#pragma once
#include <array>
#include "DrawPool.hpp"
#include "Utilities.hpp"

class Chunk
{
public:
	Chunk(glm::ivec3 t_ChunkPosition, DrawPool& t_DrawPool);

	void MeshChunk();
	//void SetChunkNeighbor(Utilities::DIRECTION t_Direction, Chunk* t_Neighbor);

	Chunk(const Chunk& t_Other) = delete;
	Chunk(Chunk&& t_Other) noexcept
		: m_BlockData(std::move(t_Other.m_BlockData)),
		  m_ChunkPosition(std::move(t_Other.m_ChunkPosition)),
		  m_DrawPool(t_Other.m_DrawPool),
		  m_BucketIDs(t_Other.m_BucketIDs)
	{
	}

	Chunk& operator=(const Chunk& t_Other) = delete;
	Chunk& operator=(Chunk&& t_Other) = delete;

private:
	// Data for voxels, 16^3
	std::array<int8_t, 4096> m_BlockData{};
	//std::array<Chunk*, 6> m_ChunkNeighbors{nullptr}; // maybe use a weak pointer

	// Position of chunk relative to other chunk
	glm::ivec3 m_ChunkPosition;
	// DrawPool this chunks visuals are part of
	DrawPool& m_DrawPool;
	// Bucket ID of supplied DrawPool
	std::array<DrawPool::BucketID, 6> m_BucketIDs;

	//Use Greedy Mesh algo and push data to Draw Pool
	void GreedyMesh();
};
