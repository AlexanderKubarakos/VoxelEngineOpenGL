#pragma once
#include "Window.hpp"

#include "OpenGL/camera.hpp"

class Game
{
public:
	void Run();
	void Stop();

private:
	bool m_Running = true;
	Window m_Window { 1200, 1200, "MyGame"};
	Camera m_Camera {{0,8,16}};
};
