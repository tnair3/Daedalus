#include <iostream>

#include "Window.h"

namespace Daedalus {
    EditorWindow::EditorWindow(int width, int height, const std::string& title)
    {
        if (!glfwInit())
        {
            std::cerr << "Failed to initialize GLFW!" << std::endl;
            return;
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        m_Window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);

        if (!m_Window)
        {
            std::cerr << "Failed to create GLFW window!" << std::endl;
            glfwTerminate();
        }
    }

    EditorWindow::~EditorWindow()
    {
        if (m_Window) glfwDestroyWindow(m_Window);
        glfwTerminate();
    }

    bool EditorWindow::ShouldClose() const { return glfwWindowShouldClose(m_Window); }

    void EditorWindow::PollEvents() { glfwPollEvents(); }

    void EditorWindow::SwapBuffers()
    {
        // Only needed for OpenGL; for Vulkan, handle this via the Swapchain
        // glfwSwapBuffers(m_Window); 
    }
}
