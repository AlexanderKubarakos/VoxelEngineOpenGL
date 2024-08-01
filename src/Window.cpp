#include "Window.hpp"
#include "Input.hpp"

#include <iostream>

void WindowSizeCallback(GLFWwindow* t_Window, int t_Width, int t_Height)
{
    glViewport(0, 0, t_Width, t_Height);
}

Window::Window(int t_W, int t_H, const char* t_Name)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_Window = glfwCreateWindow(t_W, t_H, t_Name, nullptr, nullptr);
    if (m_Window == nullptr)
    {
        std::cout << "Failed to create GLFW window\n";
        glfwTerminate();
    }
    glfwMakeContextCurrent(m_Window);
    glfwSetCursorPosCallback(m_Window, Input::MouseCallback);
    glfwSetKeyCallback(m_Window, Input::KeyCallback);
    glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetWindowSizeCallback(m_Window, WindowSizeCallback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD\n";
    }

    glViewport(0, 0, t_W, t_H);
    glEnable(GL_DEPTH_TEST);
    glfwSwapInterval(0);
}

Window::~Window()
{
    glfwTerminate();
}

glm::ivec2 Window::getExtent()
{
    glm::ivec2 size;
    glfwGetFramebufferSize(m_Window, &size.x, &size.y);
    return size;
}
