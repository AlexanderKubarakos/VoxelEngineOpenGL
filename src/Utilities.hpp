#pragma once
#include "GLFW/glfw3.h"

namespace Utilities
{
	enum DIRECTION
	{
		UP, // +z
		DOWN, // -z
		NORTH, // +y (forward)
		SOUTH, // -y (back)
		EAST, // +x (right)
		WEST // -x (left)
	};
	void ProcessFrame(GLFWwindow* t_Window);
	double GetDeltaTime();
};