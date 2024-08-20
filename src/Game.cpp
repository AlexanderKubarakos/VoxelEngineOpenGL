#include "Game.hpp"

#include <chrono>

#include "OpenGL/VAO.hpp"
#include "Chunk.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "ChunkManager.hpp"
#include "DrawPool.hpp"
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
	bool showDemoWindow = false;
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(m_Window.GetWindowPointer(), true);
	ImGui_ImplOpenGL3_Init();

	ChunkManager chunkManger;

	int verticalScale = 6;
	int horizontalScale = 32;

	for (int x = -horizontalScale; x < horizontalScale; x++)
	{
		for (int y = -verticalScale; y < verticalScale; y++)
		{
			for (int z = -horizontalScale; z < horizontalScale; z++)
			{
				chunkManger.AddChunk(glm::vec3(x, y, z));
			}
		}
	}
	//chunkManger.AddChunk(glm::vec3(0, 0, 0));
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
		
		ImGui::Checkbox("Show Demo Window", &showDemoWindow);
		if (showDemoWindow)
			ImGui::ShowDemoWindow();

		chunkManger.ShowDebugInfo();

		// Swap wireframe
		if (Input::IsKeyPressed(GLFW_KEY_X))
		{
			if (wireframe)
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			else
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			wireframe = !wireframe;
		}

		m_Camera.ProcessInput();
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), static_cast<float>(m_Window.getExtent().x) / m_Window.getExtent().y, 0.1f,10000.0f);

		chunkManger.MeshChunks();
		chunkManger.RenderChunks(m_Camera, projection);

        // End of frame
        Input::ResetInputs(m_Window); // Resets all need inputs

		// ImGUI rendering end of frame
		ImGui::End();
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