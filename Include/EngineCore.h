#pragma once

#include <GLFW/glfw3.h>
#include <string>

#include "EngineContext.h"

namespace Daedalus {
    class EngineCore
    {
        public:
            EngineCore();
            ~EngineCore();

            [[nodiscard]] const EngineGraphicsContext& GetGraphicsContext() const { return m_GraphicsContext; }
            [[nodiscard]] const OffscreenRenderTarget& GetRenderTarget() const { return m_OffscreenTarget; }

            void RecreateSwapchain(GLFWwindow* window);
            void Initialize(GLFWwindow* window);
            void Update();
            void Render();
            void Shutdown();
            bool LoadProject(const std::string& projectPath);

        private:
            [[nodiscard]] uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;
            void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

            EngineGraphicsContext m_GraphicsContext;
            OffscreenRenderTarget m_OffscreenTarget;
            VkCommandPool m_EngineCommandPool = VK_NULL_HANDLE;
            VkCommandBuffer m_EngineCommandBuffer = VK_NULL_HANDLE;
            std::string m_ActiveProjectPath;
    };
}
