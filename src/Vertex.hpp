#pragma once

struct Vertex
{
	int position;

	Vertex(int x, int y, int z) {
		assert(x <= 32 && x >= 0 && y <= 32 && y >= 0 && z <= 32 && z >= 0);
		x |= y << 6;
		x |= z << 12;
		position = x;
	}
};
