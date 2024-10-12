#pragma once
#include <string>

#include "GLFW/glfw3.h"

#include "glm/vec3.hpp"
#include <chrono>
#include <iostream>

#define ENABLE_TIMER 1

#if ENABLE_TIMER
#define TIMER_START(ID) auto timerStart##ID = std::chrono::high_resolution_clock::now();
#define TIMER_END(ID, Message) auto timerEnd##ID = std::chrono::high_resolution_clock::now(); \
	auto microseconds##ID = std::chrono::duration_cast<std::chrono::microseconds>(timerEnd##ID-timerStart##ID); \
	std::cout << (Message) << microseconds##ID.count() << " microseconds\n";
#else
#define TIMER_START(ID)
#define TIMER_END(ID, Message) 
#endif

#define VECTOR_TO_STRING(Vector) "x:" << (Vector).x << " y:" << (Vector).y << " z:" << (Vector).z

//#if _DEBUG 
#define LOG_PRINT(Message) std::cerr << Message << '\n';
#define ERROR_PRINT(Message) std::cerr << Message << '\n';
//#else
//#define LOG_PRINT(Message) 
//#define ERROR_PRINT(Message)
//#endif

namespace Utilities
{
	enum DIRECTION
	{
		UP = 0, // +y
		DOWN, // -y
		SOUTH, // +z (back)
		NORTH, // -z (forward)
		EAST, // +x (right)
		WEST // -x (left)
	};
	void ProcessFrame(GLFWwindow* t_Window);
	double GetDeltaTime();
	std::string vectorToString(glm::vec3 t_Vec);
	std::string vectorToString(glm::ivec3 t_Vec);
	struct VectorCompare {
		template <typename T>
		bool operator()(const T& lhs, const T& rhs) const {
			return lhs.x != rhs.x ?
				lhs.x < rhs.x
				: lhs.y != rhs.y ?
				lhs.y < rhs.y
				: lhs.z < rhs.z;
		}
	};
};

