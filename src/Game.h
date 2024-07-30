#pragma once
#include "Window.hpp"

#include "OpenGL/camera.hpp"
#include "OpenGL/Shader.h"

#define WIDTH 1200
#define HEIGHT 1200

class Game
{
public:
	void Run();
	void Stop();

private:
	bool m_Running = true;
	Window m_Window { WIDTH, HEIGHT, "MyGame"};
	Shader m_Shader {"src/Shaders/vertexShader.glsl", "src/Shaders/fragmentShader.glsl"};
	Camera m_Camera {{0,0,64}};
};
