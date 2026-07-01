#pragma once

#include <vulkan/vulkan.h>

#include "EngineCore.h"
#include "Window.h"

namespace Editor {
    class EditorApp
    {
        public:
            EditorApp();
            ~EditorApp();

            void Run();

        private:
            bool m_IsRunning;

            Daedalus::EditorWindow m_Window;
            Daedalus::EngineCore m_engine;

            // ImGui Vulkan Backend Requirements
            VkInstance m_VkInstance = VK_NULL_HANDLE;
            VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
            VkDevice m_Device = VK_NULL_HANDLE;
            VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
            VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
            VkRenderPass m_RenderPass = VK_NULL_HANDLE;
            VkCommandPool m_CommandPool = VK_NULL_HANDLE;
            VkCommandBuffer m_CommandBuffer = VK_NULL_HANDLE;
            VkCommandPool m_CommandPoolObj = VK_NULL_HANDLE;
            VkCommandBuffer m_CommandBufferObj = VK_NULL_HANDLE;

            void InitVulkanContext();
            void InitImGui();
            void CleanupVulkanContext();
            void CleanupImGui();

            void BeginFrame();
            void EndFrame();
    };
}
