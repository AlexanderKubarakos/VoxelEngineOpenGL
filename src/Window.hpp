#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Window
{
public:
	Window(int t_W, int t_H, const char* t_Name);
	~Window();

	bool ShouldWindowClose() const { return glfwWindowShouldClose(m_Window); }
	GLFWwindow* GetWindowPointer() const { return m_Window;  }

	Window(const Window& other) = delete;
	Window(Window&& other) noexcept = delete;
	Window& operator=(const Window& other) = delete;
	Window& operator=(Window&& other) noexcept = delete;
private:
	GLFWwindow* m_Window;
	int m_Width, m_Height;
};