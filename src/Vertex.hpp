#pragma once

struct Vertex
{
	int position;

	Vertex(int x, int y, int z) {
		assert(x < 32 && x > -1 && y < 32 && y > -1 && z < 32 && z > -1);
		x |= y << 5;
		x |= z << 10;
		position = x;
	}
};
