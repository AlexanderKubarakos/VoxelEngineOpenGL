#pragma once
#include "Window.hpp"

#include "OpenGL/Shaders/Shader.h"

#define WIDTH 800
#define HEIGHT 800

class Game
{
public:
	void Run();
	void Stop();

private:
	bool m_Running = true;
	Window m_Window { WIDTH, HEIGHT, "MyGame"};
	Shader m_Shader {"src/OpenGL/Shaders/vertexShader.glsl", "src/OpenGL/Shaders/fragmentShader.glsl"};
};
