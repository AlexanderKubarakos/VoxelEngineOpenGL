#pragma once
#include "Window.hpp"

#include "OpenGL/Shaders/Shader.h"

class Game
{
public:
	void Run();
	void Stop();

private:
	bool mRunning = true;
	Window mWindow {800, 800, "MyGame"};
	Shader mShader {"src/OpenGL/Shaders/vertexShader.glsl", "src/OpenGL/Shaders/fragmentShader.glsl"};
};
