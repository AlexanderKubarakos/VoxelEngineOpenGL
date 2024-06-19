#include "Utilities.hpp"

namespace
{
	double DT = 0.0;
	double LastFrameTime = 0.0;
}

void Utilities::ProcessFrame(GLFWwindow* t_Window)
{
	DT = glfwGetTime() - LastFrameTime;
	LastFrameTime = glfwGetTime();
}

double Utilities::GetDeltaTime()
{
	return DT;
}