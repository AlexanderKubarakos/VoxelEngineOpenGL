#pragma once
#include <array>

#include "OpenGL/VBO.h"

class Chunk
{
public:
	void DrawChunk(); // binds its VBO, and 
private:
	// maybe use enum for material list
	std::array<int, 4096> mBlockData;
	VBO mMeshData;
};

// Hold data
// Hold a mesh
// Allow Re-meshing
