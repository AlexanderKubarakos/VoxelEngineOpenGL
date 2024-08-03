#pragma once
#include <string>

#include "GLFW/glfw3.h"

#include "glm/vec3.hpp"

namespace Utilities
{
	enum DIRECTION
	{
		UP = 0, // +y
		DOWN, // -y
		SOUTH, // +z (back)
		NORTH, // -z (forward)
		EAST, // +x (right)
		WEST // -x (left)
	};
	void ProcessFrame(GLFWwindow* t_Window);
	double GetDeltaTime();
	std::string vectorToString(glm::vec3 t_Vec);
	std::string vectorToString(glm::ivec3 t_Vec);
};