#pragma once

#include <GLFW/glfw3.h>

#include "EngineContext.h"

namespace Daedalus {
    class EngineCore
    {
        public:
            EngineCore();
            ~EngineCore();

            const EngineGraphicsContext& GetGraphicsContext() const { return m_GraphicsContext; }

            void RecreateSwapchain(GLFWwindow* window);
            void Initialize(GLFWwindow* window);
            void Update();
            void Shutdown();

        private:
            EngineGraphicsContext m_GraphicsContext;

            void CreateMinimalVulkanInstance();
    };
}
