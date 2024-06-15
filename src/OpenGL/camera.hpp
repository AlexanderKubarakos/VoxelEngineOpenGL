#pragma once
#include <glm/glm.hpp>

#include "GLFW/glfw3.h"

#include "glm/ext/matrix_transform.hpp"

class Camera
{
public:
	Camera(glm::vec3 t_Position);
	glm::mat4 GetViewMatrix() const { return glm::lookAt(m_CameraPos, m_CameraFront + m_CameraPos, m_CameraUp); }
	void ProcessInput(GLFWwindow* t_Window, double t_DeltaTime)
	{
		if (glfwGetKey(t_Window, GLFW_KEY_W) == GLFW_PRESS)
			m_CameraPos += m_MoveSpeed * m_CameraFront;
		if (glfwGetKey(t_Window, GLFW_KEY_S) == GLFW_PRESS)
			m_CameraPos -= m_MoveSpeed * m_CameraFront;
		if (glfwGetKey(t_Window, GLFW_KEY_A) == GLFW_PRESS)
			m_CameraPos -= glm::normalize(glm::cross(m_CameraFront, m_CameraUp)) * m_MoveSpeed;
		if (glfwGetKey(t_Window, GLFW_KEY_D) == GLFW_PRESS)
			m_CameraPos += glm::normalize(glm::cross(m_CameraFront, m_CameraUp)) * m_MoveSpeed;
	}

private:
	float m_MoveSpeed = 1.0f;
	glm::vec3 m_CameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
	glm::vec3 m_CameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 m_CameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
};
