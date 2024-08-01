#pragma once
#include "Window.hpp"

#include "OpenGL/camera.hpp"
#include "OpenGL/Shader.hpp"

class Game
{
public:
	void Run();
	void Stop();

private:
	bool m_Running = true;
	Window m_Window { 1200, 1200, "MyGame"};
	Shader m_Shader {"src/Shaders/vertexShader.glsl", "src/Shaders/fragmentShader.glsl"};
	Camera m_Camera {{0,0,64}};
};
