#pragma once
#include "Window.hpp"

#include "OpenGL/Shaders/Shader.h"

class Game
{
public:
	void Run();
	void Stop();

private:
	bool m_Running = true;
	Window m_Window {800, 800, "MyGame"};
	Shader m_Shader {"src/OpenGL/Shaders/vertexShader.glsl", "src/OpenGL/Shaders/fragmentShader.glsl"};
};
