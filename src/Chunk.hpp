#pragma once
#include <array>

#include "OpenGL/VAO.h"
#include "OpenGL/VBO.h"

#include "Utilities.hpp"

class Chunk
{
public:
	void RenderChunk(VAO& t_ChunkVAO); // binds its VBO, and
	void MeshChunk();
	void SetChunkNeighbor(Utilities::DIRECTION t_Direction, Chunk* t_Neighbor);
private:
	// maybe use enum for material list
	std::array<int, 4096> m_BlockData{};
	std::array<Chunk*, 6> m_ChunkNeighbors{nullptr}; // maybe use a weak pointer
	int m_MeshLength{0};
	VBO m_MeshData{};
};

// Hold data
// Hold a mesh
// Allow Re-meshing
