#pragma once
#include <array>
#include "DrawPool.hpp"
#include "Utilities.hpp"

class Chunk
{
public:
	Chunk(glm::vec3 t_ChunkPosition, DrawPool& t_DrawPool);

	void MeshChunk();
	//void SetChunkNeighbor(Utilities::DIRECTION t_Direction, Chunk* t_Neighbor);

	Chunk(const Chunk& t_Other) = delete;
	Chunk(Chunk&& t_Other) noexcept
		: m_BlockData(std::move(t_Other.m_BlockData)),
		  m_ChunkPosition(std::move(t_Other.m_ChunkPosition)),
		  m_DrawPool(t_Other.m_DrawPool),
		  m_BucketID(t_Other.m_BucketID)
	{
	}

	Chunk& operator=(const Chunk& t_Other) = delete;
	Chunk& operator=(Chunk&& t_Other) = delete;

private:
	// Data for voxels, 16^3
	std::array<int8_t, 4096> m_BlockData{};
	//std::array<Chunk*, 6> m_ChunkNeighbors{nullptr}; // maybe use a weak pointer

	// Position of chunk relative to other chunk
	glm::vec3 m_ChunkPosition;
	// DrawPool this chunks visuals are part of
	DrawPool& m_DrawPool;
	// Bucket ID of supplied DrawPool
	DrawPool::BucketID m_BucketID;

	//Use Greedy Mesh algo and push data to Draw Pool
	void GreedyMesh();
};
