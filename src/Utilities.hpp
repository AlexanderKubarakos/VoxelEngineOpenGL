#pragma once
#include "GLFW/glfw3.h"

namespace Utilities
{
	enum DIRECTION
	{
		UP,
		DOWN,
		NORTH,
		EAST,
		SOUTH,
		WEST
	};
	void ProcessFrame(GLFWwindow* t_Window);
	double GetDeltaTime();
};