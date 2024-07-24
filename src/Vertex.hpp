#pragma once

#include "glm/vec3.hpp"

struct Vertex
{
	float position[3];
	float normal[3];
	float color[4];

	Vertex(glm::vec3 p, glm::vec3 n, glm::vec3 c) {
		position[0] = p.x;
		position[1] = p.y;
		position[2] = p.z;
		normal[0] = n.x;
		normal[1] = n.y;
		normal[2] = n.z;
		color[0] = c.x;
		color[1] = c.y;
		color[2] = c.z;
		color[3] = 1.0;
	}
};
