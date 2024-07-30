#pragma once
#include "GLFW/glfw3.h"

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
};