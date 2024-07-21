#pragma once
#include <algorithm>

#include <glm/glm.hpp>

#include <GLFW/glfw3.h>
#include "Utilities.hpp"
#include "Input.hpp"

#include "glm/ext/matrix_transform.hpp"

class Camera
{
public:
	Camera(glm::vec3 t_Position, glm::vec3 t_Up = glm::vec3(0.0f, 1.0f, 0.0f), float t_Yaw = -90.0f, float t_Pitch = 0.0f)
		: m_MoveSpeed(15.0f), m_Sensitivity(0.3f), m_CameraPos(t_Position), m_CameraFront(glm::vec3(0.0f, 0.0f, -1.0f))
	, m_CameraUp(t_Up), m_Yaw(t_Yaw), m_Pitch(t_Pitch) {}
	glm::mat4 GetViewMatrix() const { return glm::lookAt(m_CameraPos, m_CameraFront + m_CameraPos, m_CameraUp); }
	void ProcessInput()
	{
		float dt = static_cast<float>(Utilities::GetDeltaTime());
		if (Input::IsKeyDown(GLFW_KEY_W))
		{
			m_CameraPos += dt * m_MoveSpeed * m_CameraFront;
		}
		if (Input::IsKeyDown(GLFW_KEY_S))
		{
			m_CameraPos -= dt * m_MoveSpeed * m_CameraFront;
		}
		if (Input::IsKeyDown(GLFW_KEY_A))
		{
			m_CameraPos -= dt * glm::normalize(glm::cross(m_CameraFront, m_CameraUp)) * m_MoveSpeed;
		}
		if (Input::IsKeyDown(GLFW_KEY_D))
		{
			m_CameraPos += dt * glm::normalize(glm::cross(m_CameraFront, m_CameraUp)) * m_MoveSpeed;
		}

		// Update rotation from mouse
		glm::vec2 offset = Input::GetMouseOffset();
		offset.x *= m_Sensitivity;
		offset.y *= m_Sensitivity;

		m_Yaw += offset.x;
		m_Pitch += offset.y;

		m_Pitch = std::clamp(m_Pitch, -89.0f, 89.0f);

		UpdateCameraVectors();
	}

private:
	float m_MoveSpeed;
	float m_Sensitivity;
	glm::vec3 m_CameraPos;
	glm::vec3 m_CameraFront;
	glm::vec3 m_CameraUp;
	glm::vec3 m_CameraRight;
	float m_Yaw;
	float m_Pitch;

	void UpdateCameraVectors()
	{
		// calculate the new Front vector
		glm::vec3 front;
		front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
		front.y = sin(glm::radians(m_Pitch));
		front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
		m_CameraFront = glm::normalize(front);
		// also re-calculate the Right and Up vector
		m_CameraRight = glm::normalize(glm::cross(m_CameraFront, {0,1,0}));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		m_CameraUp = glm::normalize(glm::cross(m_CameraRight, m_CameraFront));
	}
};