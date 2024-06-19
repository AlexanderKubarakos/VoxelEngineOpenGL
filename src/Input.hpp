#pragma once
#include "GLFW/glfw3.h"

#include "glm/glm.hpp"

namespace Input
{
	enum KeyState
	{
		UP = 0,// Key is not depressed
		DOWN = 1, // Key is held down
		PRESSED = 2 // Key is held down AND it was just pressed
	};

	glm::vec2 GetMouseOffset();
	bool IsKeyUp(int t_GLFWKeyCode);
	bool IsKeyDown(int t_GLFWKeyCode);
	bool IsKeyPressed(int t_GLFWKeyCode);

	void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	void MouseCallback(GLFWwindow* window, double xposIn, double yposIn);
	void ResetMouseDifference();
}
