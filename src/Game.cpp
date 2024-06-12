#include "Game.h"

void Game::Stop()
{
	mRunning = false;
}

void Game::Run()
{
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	while (mRunning)
	{
		glClear(GL_COLOR_BUFFER_BIT);
		// Game code here
		
		

		glfwSwapBuffers(mWindow.GetWindowPointer());
		glfwPollEvents();
		if (mWindow.ShouldWindowClose())
		{
			Stop();
		}
	}
}

