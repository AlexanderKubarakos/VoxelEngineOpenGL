#pragma once
#include <array>

#include "OpenGL/VAO.h"
#include "OpenGL/VBO.h"

#include "Utilities.hpp"

#include "glm/glm.hpp"

#include "OpenGL/Shaders/Shader.h"

class Chunk
{
public:
	Chunk(glm::vec3 t_ChunkPosition);

	void RenderChunk(VAO& t_ChunkVAO, Shader& t_Shader); // binds its VBO, and
	void MeshChunk();
	void SetChunkNeighbor(Utilities::DIRECTION t_Direction, Chunk* t_Neighbor);

	Chunk(const Chunk& t_Other) = delete;

	Chunk(Chunk&& t_Other) noexcept
		: m_BlockData(std::move(t_Other.m_BlockData)),
		  m_ChunkNeighbors(std::move(t_Other.m_ChunkNeighbors)),
		  m_ChunkPosition(std::move(t_Other.m_ChunkPosition)),
		  m_MeshLength(t_Other.m_MeshLength),
		  m_MeshData(std::move(t_Other.m_MeshData))
	{
	}

	Chunk& operator=(const Chunk& t_Other) = delete;

	Chunk& operator=(Chunk&& t_Other) noexcept
	{
		if (this == &t_Other)
			return *this;
		m_BlockData = std::move(t_Other.m_BlockData);
		m_ChunkNeighbors = std::move(t_Other.m_ChunkNeighbors);
		m_ChunkPosition = std::move(t_Other.m_ChunkPosition);
		m_MeshLength = t_Other.m_MeshLength;
		m_MeshData = std::move(t_Other.m_MeshData);
		return *this;
	}
	// TODO: make private, public just for testing...
	void GreedyMesh();
private:
	// maybe use enum for material list
	std::array<int, 4096> m_BlockData{};
	std::array<Chunk*, 6> m_ChunkNeighbors{nullptr}; // maybe use a weak pointer

	glm::vec3 m_ChunkPosition;
	int m_MeshLength{0};
	VBO m_MeshData{};

	
};

// Hold data
// Hold a mesh
// Allow Re-meshing
