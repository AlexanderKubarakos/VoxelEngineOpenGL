#include "Game.h"

#include <iostream>

#include "OpenGL/VAO.h"
#include "Chunk.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

void Game::Stop()
{
	m_Running = false;
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void Game::Run()
{
	VAO chunkVAO;
	
	chunkVAO.AddAttribute(0, 0, 3, GL_FLOAT, GL_FALSE, 0); // Position Attribute
	chunkVAO.AddAttribute(1, 0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3); // Position Attribute

	std::vector<Chunk> chunks;

	for (int x = -1; x <= 0; x++)
	{
		for (int y = -1; y <= 0; y++)
		{
			for (int z = -1; z <= 0; z++)
			{
				//chunks.emplace_back(glm::vec3(x,y,z));
				//chunks.back().MeshChunk();
			}
		}
	}

	chunks.emplace_back(glm::vec3(0,0,0));
	chunks.back().MeshChunk();
	chunks.back().GreedyMesh();

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(m_Window.GetWindowPointer(), true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
	ImGui_ImplOpenGL3_Init();

	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	bool wireframe = false;
	while (m_Running)
	{
        // Start Frame
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear buffers
        Utilities::ProcessFrame(m_Window.GetWindowPointer()); // Do start of frame actions, ex. calculate delta time

		// ImGUI start of frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Debug");
		ImGui::Text("Delta Time: %fms", Utilities::GetDeltaTime() * 1000);
		ImGui::Text("FPS: %.2f", 1 / Utilities::GetDeltaTime());
		ImGui::End();
		// Swap wireframe
		if (Input::IsKeyPressed(GLFW_KEY_X))
		{
			if (wireframe)
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			else
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			wireframe = !wireframe;
		}

		if (Input::IsKeyPressed(GLFW_KEY_H))
			chunks.back().MeshChunk();
		if (Input::IsKeyPressed(GLFW_KEY_J))
			chunks.back().GreedyMesh();

		m_Camera.ProcessInput();

        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), static_cast<float>(WIDTH) / HEIGHT, 0.1f, 250.0f);

        glm::mat4 MVP = projection * m_Camera.GetViewMatrix() * model;

		m_Shader.Use();
		m_Shader.SetMatrix4f("MVP", MVP);
		chunkVAO.Bind();
		// Draw chunks here
		for (Chunk& chunk : chunks)
		{
			chunk.RenderChunk(chunkVAO, m_Shader);
		}

        // End of frame
        Input::ResetInputs(m_Window); // Resets all need inputs

		// ImGUI rendering end of frame
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(m_Window.GetWindowPointer()); // Swap buffers
        glfwPollEvents(); // Poll events
		if (m_Window.ShouldWindowClose() || Input::IsKeyDown(GLFW_KEY_ESCAPE)) // Close game if needed
		{
			Stop();
		}
	}
}

