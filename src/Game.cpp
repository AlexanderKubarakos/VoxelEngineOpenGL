#include "Game.h"

#include "OpenGL/VAO.h"

void Game::Stop()
{
	m_Running = false;
}

void Game::Run()
{

	std::vector<float> vertices = {
	-0.5f, -0.5f, 0.0f,
	 0.5f, -0.5f, 0.0f,
	 0.0f,  0.5f, 0.0f
	};

	VAO chunkVAO;
	VBO vbo;

	vbo.SetBufferData(vertices);
	chunkVAO.AddAttribute(0, 0, 3, GL_FLOAT, GL_FALSE); // Position Attribute
	chunkVAO.BindVertexBuffer(vbo, 0, 0, 3 * sizeof(float));

	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

	while (m_Running)
	{
		glClear(GL_COLOR_BUFFER_BIT);

		m_Shader.Use();
		chunkVAO.Bind();
		glDrawArrays(GL_TRIANGLES, 0, 3);

		glfwSwapBuffers(m_Window.GetWindowPointer());
		glfwPollEvents();
		if (m_Window.ShouldWindowClose())
		{
			Stop();
		}
	}
}

