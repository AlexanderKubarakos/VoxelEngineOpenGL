#pragma once
#include <iostream>
#include <cassert>
struct FaceVertex
{
	uint32_t position;

	FaceVertex(int x, int y, int z, int lengthHorizontal, int lengthVertical, int type) {
		assert(x < 32 && x >= 0 && y < 32 && y >= 0 && z < 32 && z >= 0 && lengthHorizontal < 32 && lengthHorizontal >= 0 && lengthVertical < 32 && lengthVertical >= 0);
		x |= y << 5;
		x |= z << 10;
		x |= lengthHorizontal << 15;
		x |= lengthVertical << 20; 
		x |= type << 25;
		position = x;
	}

	void debugUnpack() const
	{
		uint32_t x = position & 31;
		uint32_t y = (position >> 5) & 31;
		uint32_t z = (position >> 10) & 31;
		uint32_t lengthX = (position >> 15) & 31;
		uint32_t lengthY = (position >> 20) & 31;
		std::cout << "x: " << x << " y: " << y << " z: " << z << " lengthX: " << lengthX << " lengthY: " << lengthY << "\n";
		return;
	}
};
