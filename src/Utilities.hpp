#pragma once
#include "GLFW/glfw3.h"

namespace Utilities
{
	void ProcessFrame(GLFWwindow* t_Window);
	double GetDeltaTime();
};