#pragma once
#include "Window.hpp"

#include "GLFW/glfw3.h"

#include "glm/glm.hpp"

namespace Input
{
	enum KeyState
	{
		UP = 0,// Key is not depressed
		PRESSED = 1, // Key is held down AND it was just pressed
		DOWN = 2 // Key is held down
	};

	glm::vec2 GetMouseOffset();
	bool IsKeyUp(int t_GLFWKeyCode);
	bool IsKeyDown(int t_GLFWKeyCode);
	bool IsKeyPressed(int t_GLFWKeyCode);
	bool IsMouseButtonDown(int t_GLFWKeyCode);
	bool IsMouseButtonPressed(int t_GLFWKeyCode);

	void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	void MouseCallback(GLFWwindow* window, double xposIn, double yposIn);
	void MouseButtonCallback(GLFWwindow* t_Window, int t_Button, int t_Action, int t_Mods);
	void ResetInputs(Window& t_Window);
}
