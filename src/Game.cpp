#include "Game.h"

void Game::Stop()
{
	mRunning = false;
}

void Game::Run()
{
	while (mRunning && !window.ShouldWindowClose())
	{
		// Game code here


		glfwSwapBuffers(window.GetWindowPointer());
		glfwPollEvents();
	}
}

