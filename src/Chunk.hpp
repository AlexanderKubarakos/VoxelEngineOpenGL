#pragma once
#include <array>

#include "OpenGL/VBO.h"

class Chunk
{
public:
	void DrawChunk(); // binds its VBO, and 
private:
	// maybe use enum for material list
	std::array<int, 4096> m_BlockData;
	VBO m_MeshData;
};

// Hold data
// Hold a mesh
// Allow Re-meshing
