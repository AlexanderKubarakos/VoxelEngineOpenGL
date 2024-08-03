#include "Utilities.hpp"

#include <sstream>

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

std::string Utilities::vectorToString(glm::vec3 t_Vec)
{
	std::ostringstream ss;
	ss << "x: " << t_Vec.x << " y: " << t_Vec.y << " z: " << t_Vec.z << '\n';
	return ss.str();
}

std::string Utilities::vectorToString(glm::ivec3 t_Vec)
{
	std::ostringstream ss;
	ss << "x: " << t_Vec.x << " y: " << t_Vec.y << " z: " << t_Vec.z << '\n';
	return ss.str();
}
