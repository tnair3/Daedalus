#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <string>

namespace Daedalus {

    class EditorWindow {
    public:
        EditorWindow(int width, int height, const std::string& title);
        ~EditorWindow();

        bool ShouldClose() const;
        void PollEvents();
        void SwapBuffers();

    private:
        GLFWwindow* m_Window;
    };

}