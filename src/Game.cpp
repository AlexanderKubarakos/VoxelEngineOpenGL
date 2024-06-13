#include "Game.h"

#include "OpenGL/VAO.h"

void Game::Stop()
{
	mRunning = false;
}

void Game::Run()
{

	std::vector<float> vertices = {
	-0.5f, -0.5f, 0.0f,
	 0.5f, -0.5f, 0.0f,
	 0.0f,  0.5f, 0.0f
	};

	VAO vao;
	VBO vbo;

	vao.Bind();
	vao.AddAttribute(0, 3, GL_FLOAT, GL_FALSE);

	vbo.Bind();
	vbo.SetBufferData(vertices);
	vbo.BindBuffer(0, 0, 3 * sizeof(float));

	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

	while (mRunning)
	{
		glClear(GL_COLOR_BUFFER_BIT);

		mShader.Use();
		glDrawArrays(GL_TRIANGLES, 0, 3);

		glfwSwapBuffers(mWindow.GetWindowPointer());
		glfwPollEvents();
		if (mWindow.ShouldWindowClose())
		{
			Stop();
		}
	}
}

