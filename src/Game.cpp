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
	constexpr int verticalScale = 5;
	constexpr int horizontalScale = 5;
	constexpr int totalChunkCount = (verticalScale * 2 + 1) * (horizontalScale * 2 + 1) * (horizontalScale * 2 + 1);
	
	//chunkManger.AddChunk(glm::vec3(0,0,0));
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	bool wireframe = false;

	while (m_Running)
	{
        // Start Frame
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear buffers
        Utilities::ProcessFrame(m_Window.GetWindowPointer()); // Do start of frame actions, ex. calculate delta time

		// Swap wireframe
		if (Input::IsKeyPressed(GLFW_KEY_X))
		{
			if (wireframe)
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			else
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			wireframe = !wireframe;
		}

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

		chunkManger.LoadUnloadAroundPlayer(m_Camera);
		chunkManger.MeshChunks();
		chunkManger.ShowDebugInfo();

		m_Camera.ProcessInput();
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), static_cast<float>(m_Window.getExtent().x) / static_cast<float>(m_Window.getExtent().y), 0.1f,10000.0f);


		chunkManger.RenderChunks(m_Camera, projection);

        // End of frame
        Input::ResetInputs(m_Window); // Resets all need inputs

		// ImGUI rendering end of frame
		ImGui::End();
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		LOG_PRINT("HI");
		glfwSwapBuffers(m_Window.GetWindowPointer()); // Swap buffers
        glfwPollEvents(); // Poll events
		LOG_PRINT("BYE")
		if (m_Window.ShouldWindowClose() || Input::IsKeyDown(GLFW_KEY_ESCAPE)) // Close game if needed
		{
			Stop();
		}
	}
}