#pragma once
#include <array>

#include "OpenGL/VAO.h"
#include "OpenGL/Buffer.h"

#include "Utilities.hpp"

#include "glm/glm.hpp"

#include "OpenGL/Shaders/Shader.h"
#include "DrawPool.hpp"

class Chunk
{
public:
	Chunk(glm::vec3 t_ChunkPosition);

	void MeshChunk(DrawPool& t_DrawPool);
	//void SetChunkNeighbor(Utilities::DIRECTION t_Direction, Chunk* t_Neighbor);

	Chunk(const Chunk& t_Other) = delete;
	Chunk(Chunk&& t_Other) noexcept
		: m_BlockData(std::move(t_Other.m_BlockData)),
		  //m_ChunkNeighbors(std::move(t_Other.m_ChunkNeighbors)),
		  m_ChunkPosition(std::move(t_Other.m_ChunkPosition))
	{
	}

	Chunk& operator=(const Chunk& t_Other) = delete;
	Chunk& operator=(Chunk&& t_Other) noexcept
	{
		if (this == &t_Other)
			return *this;
		m_BlockData = std::move(t_Other.m_BlockData);
		//m_ChunkNeighbors = std::move(t_Other.m_ChunkNeighbors);
		m_ChunkPosition = std::move(t_Other.m_ChunkPosition);
		return *this;
	}

private:
	// maybe use enum for material list
	std::array<int8_t, 4096> m_BlockData{};
	//std::array<Chunk*, 6> m_ChunkNeighbors{nullptr}; // maybe use a weak pointer

	glm::vec3 m_ChunkPosition;

	DrawPool::BucketID m_BucketID;

	void GreedyMesh(DrawPool& t_DrawPool);
};
