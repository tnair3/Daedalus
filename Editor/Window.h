#pragma once

#define GLFW_INCLUDE_NONE

#include <GLFW/glfw3.h>
#include <string>

namespace Daedalus {
    class EditorWindow
    {
        public:
            EditorWindow(int width, int height, const std::string& title);
            ~EditorWindow();

            EditorWindow(const EditorWindow&) = delete;
            EditorWindow& operator=(const EditorWindow&) = delete;

            bool ShouldClose() const;
            void PollEvents();
            void SwapBuffers();

            bool WasFramebufferResized() const { return m_FramebufferResized; }
            void ResetFramebufferResizedFlag() { m_FramebufferResized = false; }

            [[nodiscard]] GLFWwindow* GetNativeWindow() const { return m_Window; }

        private:
            GLFWwindow* m_Window;
            bool m_FramebufferResized = false;

            static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);
    };
}
