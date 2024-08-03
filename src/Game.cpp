#include "Game.hpp"

#include <chrono>

#include "OpenGL/VAO.hpp"
#include "Chunk.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

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

// TODO: add debug drawing, so that you can declare in a macro maybe LineSegment(glm::vec3, glm::vec3, glm::vec3 color);

void Game::Run()
{
	bool showDemoWindow = false;
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(m_Window.GetWindowPointer(), true);
	ImGui_ImplOpenGL3_Init();

	DrawPool pool{ 8*8*8*6, 8096 };

	std::vector<Chunk> chunks;

	chunks.emplace_back(glm::vec3(0, 0, 0), pool);
	chunks.back().MeshChunk();

	chunks.emplace_back(glm::vec3(1, 0, 0), pool);
	chunks.back().MeshChunk();

	chunks.emplace_back(glm::vec3(-1, 0, 0), pool);
	chunks.back().MeshChunk();

	chunks.emplace_back(glm::vec3(0, 2, 0), pool);
	chunks.back().MeshChunk();

	chunks.emplace_back(glm::vec3(2, 2, 0), pool);
	chunks.back().MeshChunk();

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

		pool.Debug();

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

        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), static_cast<float>(m_Window.getExtent().x) / m_Window.getExtent().y, 0.1f,16.0f * 16.0f);
        glm::mat4 MVP = projection * m_Camera.GetViewMatrix() * model;

		pool.Render(m_Shader, MVP);

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

// TODO: add normal information
// and add some lighting

