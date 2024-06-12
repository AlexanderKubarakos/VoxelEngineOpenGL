#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Window
{
public:
	Window(int tW, int tH, const char* tName);
	~Window();

	bool ShouldWindowClose() const { return glfwWindowShouldClose(mWindow); }
	GLFWwindow* GetWindowPointer() const { return mWindow;  }

	Window(const Window& other) = delete;
	Window(Window&& other) noexcept = delete;
	Window& operator=(const Window& other) = delete;
	Window& operator=(Window&& other) noexcept = delete;

private:
	GLFWwindow* mWindow;
	int mWidth, mHeight;
};