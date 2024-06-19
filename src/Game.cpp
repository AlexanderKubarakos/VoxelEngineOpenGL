#include "Game.h"

#include "OpenGL/VAO.h"
#include "Chunk.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

void Game::Stop()
{
	m_Running = false;
}

void Game::Run()
{
	VAO chunkVAO;
	
	chunkVAO.AddAttribute(0, 0, 3, GL_FLOAT, GL_FALSE); // Position Attribute

	Chunk chunk;
	chunk.MeshChunk();

	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	bool wireframe = false;
	while (m_Running)
	{
        // Start Frame
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear buffers
        Utilities::ProcessFrame(m_Window.GetWindowPointer()); // Do start of frame actions, ex. calculate delta time
        m_Camera.ProcessInput();
		if (Input::IsKeyPressed(GLFW_KEY_X))
		{
			if (wireframe)
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}
			else
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}
			wireframe = !wireframe;
		}


        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), static_cast<float>(WIDTH) / HEIGHT, 0.1f, 100.0f);

        model = glm::translate(model, { 0, 0, -5 });

        glm::mat4 MVP = projection * m_Camera.GetViewMatrix() * model;

		m_Shader.Use();
		m_Shader.SetMatrix4f("MVP", MVP);
		chunkVAO.Bind();
		// Draw chunks here
		chunk.RenderChunk(chunkVAO);

        // End of frame
        Input::ResetInputs(); // Resets all need inputs
		glfwSwapBuffers(m_Window.GetWindowPointer()); // Swap buffers
        glfwPollEvents(); // Poll events
		if (m_Window.ShouldWindowClose()) // Close game if needed
		{
			Stop();
		}
	}
}

