#include "Input.hpp"

namespace Input
{
	KeyState keyStates[GLFW_KEY_LAST];
	float lastX = 0, lastY = 0;
	bool firstTime = true;
	glm::vec2 mouseDifference = { 0,0 };
}

bool Input::IsKeyUp(int t_GLFWKeyCode)
{
	return keyStates[t_GLFWKeyCode] != KeyState::DOWN && keyStates[t_GLFWKeyCode] != KeyState::PRESSED;
}

bool Input::IsKeyDown(int t_GLFWKeyCode)
{
	return keyStates[t_GLFWKeyCode] == KeyState::DOWN || keyStates[t_GLFWKeyCode] == KeyState::PRESSED;
}

bool Input::IsKeyPressed(int t_GLFWKeyCode)
{
	return keyStates[t_GLFWKeyCode] == KeyState::PRESSED;
}

void Input::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	keyStates[key] = static_cast<KeyState>(action);
}

glm::vec2 Input::GetMouseOffset()
{
    return mouseDifference;
}

void Input::MouseCallback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstTime)
    {
        lastX = xpos;
        lastY = ypos;
        firstTime = false;
    }

    mouseDifference.x = xpos - lastX;
    mouseDifference.y = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;
}

void Input::ResetInputs()
{
    mouseDifference = {0,0};
    for (auto& keyState : keyStates)
    {
	    if (keyState == PRESSED)
	    {
		    keyState = DOWN;
	    }
    }
}
