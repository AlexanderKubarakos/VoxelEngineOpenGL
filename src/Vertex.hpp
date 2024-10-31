#pragma once
#include <iostream>

struct FaceVertex
{
	uint32_t position;

	FaceVertex(int x, int y, int z, int lengthHorizontal, int lengthVertical, int type) {
		assert(x < 32 && x >= 0 && y < 32 && y >= 0 && z < 32 && z >= 0);
		x |= y << 6;
		x |= z << 12;
		x |= lengthHorizontal << 18;
		x |= lengthVertical << 24;
		x |= type << 30;
		position = x;
	}

	void debugUnpack() const
	{
		uint32_t x = position & 63;
		uint32_t y = (position >> 6) & 63;
		uint32_t z = (position >> 12) & 63;
		uint32_t lengthX = (position >> 18) & 63;
		uint32_t lengthY = (position >> 24) & 63;
		std::cout << "x: " << x << " y: " << y << " z: " << z << " lengthX: " << lengthX << " lengthY: " << lengthY << "\n";
		return;
	}
};
