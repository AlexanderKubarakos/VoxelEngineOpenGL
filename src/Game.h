#pragma once
#include "Window.hpp"

class Game
{
public:
	void Run();
	void Stop();

private:
	bool mRunning = true;
	Window window {800, 800, "MyGame"};
};
