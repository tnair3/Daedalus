#pragma once

#include <vulkan/vulkan.h>
#include <imgui.h>

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

            Daedalus::EngineCore m_engine;
            Daedalus::EditorWindow m_Window;
            VkCommandPool m_ImGuiCommandPool = VK_NULL_HANDLE;
            VkCommandBuffer m_ImGuiCommandBuffer = VK_NULL_HANDLE;
            ImTextureID m_ViewportTextureID = 0;
            VkImageView m_CurrentViewportImageView = VK_NULL_HANDLE;

            void InitImGui();
            void InitVulkanContext();
            void CleanupImGui();
            void CleanupVulkanContext();

            void BeginFrame();
            void EndFrame();
    };
}
