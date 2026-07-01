#include <Windows.h>
#include <iostream>
#include <tchar.h>
#include <stdexcept>

#include "Main.h"
#include "Window.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

namespace Editor {
    EditorApp::EditorApp() : m_IsRunning(true), m_Window(1280, 720, "Daedalus Engine Editor - v1.0.0")
    {
        m_engine.Initialize(m_Window.GetNativeWindow());
        InitImGui();
    }

    EditorApp::~EditorApp()
    {
        const auto& graphicsContext = m_engine.GetGraphicsContext();

        if (graphicsContext.Device != VK_NULL_HANDLE) { vkDeviceWaitIdle(graphicsContext.Device); }

        CleanupImGui();
    }

    void EditorApp::InitImGui()
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForVulkan(m_Window.GetNativeWindow(), true);

        const auto& graphicsContext = m_engine.GetGraphicsContext();

        // Configure Vulkan rendering backend utilizing the struct parameters
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = graphicsContext.Instance;
        init_info.PhysicalDevice = graphicsContext.PhysicalDevice;
        init_info.Device = graphicsContext.Device;
        init_info.QueueFamily = graphicsContext.QueueFamilyIndex;
        init_info.Queue = graphicsContext.GraphicsQueue;
        init_info.DescriptorPool = graphicsContext.DescriptorPool;
        init_info.MinImageCount = 2;
        init_info.ImageCount = 2;
        init_info.Allocator = nullptr;

        init_info.PipelineInfoMain.RenderPass = graphicsContext.RenderPass;
        init_info.PipelineInfoMain.Subpass = 0;
        init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

        if (!ImGui_ImplVulkan_Init(&init_info)) { throw std::runtime_error("EditorApp: Failed to initialize ImGui Vulkan backend!"); }

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = graphicsContext.QueueFamilyIndex;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        if (vkCreateCommandPool(graphicsContext.Device, &poolInfo, nullptr, &m_CommandPoolObj) != VK_SUCCESS) { throw std::runtime_error("EditorApp: Failed to create command pool!"); }

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_CommandPoolObj;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        if (vkAllocateCommandBuffers(graphicsContext.Device, &allocInfo, &m_CommandBufferObj) != VK_SUCCESS) { throw std::runtime_error("EditorApp: Failed to allocate command buffer!"); }
    }

    void EditorApp::BeginFrame()
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void EditorApp::EndFrame()
    {
        ImGui::Render();

        ImDrawData* draw_data = ImGui::GetDrawData();
        if (!draw_data) return;

        const auto& graphicsContext = m_engine.GetGraphicsContext();

        uint32_t imageIndex = 0;
        VkResult result = vkAcquireNextImageKHR(
            graphicsContext.Device,
            graphicsContext.Swapchain,
            UINT64_MAX,
            VK_NULL_HANDLE,
            graphicsContext.AcquireFence,
            &imageIndex
        );

        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) return;

        vkWaitForFences(graphicsContext.Device, 1, &graphicsContext.AcquireFence, VK_TRUE, UINT64_MAX);
        vkResetFences(graphicsContext.Device, 1, &graphicsContext.AcquireFence);

        vkResetCommandBuffer(m_CommandBufferObj, 0);
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(m_CommandBufferObj, &beginInfo);

        VkClearValue clearColor = {{{0.1f, 0.1f, 0.1f, 1.0f}}};

        VkRenderPassBeginInfo rpInfo{};
        rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        rpInfo.renderPass = graphicsContext.RenderPass;
        rpInfo.framebuffer = graphicsContext.Framebuffers[imageIndex];
        rpInfo.renderArea.offset = {0, 0};
        rpInfo.renderArea.extent = graphicsContext.SwapchainExtent;
        rpInfo.clearValueCount = 1;
        rpInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(m_CommandBufferObj, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

        ImGui_ImplVulkan_RenderDrawData(draw_data, m_CommandBufferObj);

        vkCmdEndRenderPass(m_CommandBufferObj);
        vkEndCommandBuffer(m_CommandBufferObj);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_CommandBufferObj;

        vkQueueSubmit(graphicsContext.GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(graphicsContext.GraphicsQueue);

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &graphicsContext.Swapchain;
        presentInfo.pImageIndices = &imageIndex;

        vkQueuePresentKHR(graphicsContext.GraphicsQueue, &presentInfo);
    }

    void EditorApp::Run()
    {
        while (m_IsRunning && !m_Window.ShouldClose())
        {
            m_Window.PollEvents();

            BeginFrame();

            ImGui::Begin("Daedalus Hierarchy");

            ImVec2 windowSize = ImGui::GetWindowSize();

            const char* textStr = "Test";
            ImVec2 textSize = ImGui::CalcTextSize(textStr);
            float textPosX = (windowSize.x - textSize.x) * 0.5f;
            float textPosY = (windowSize.y * 0.5f) - textSize.y;

            ImGui::SetCursorPosX(textPosX);
            ImGui::SetCursorPosY(textPosY);
            ImGui::Text("%s", textStr);

            const char* buttonStr = "Click Me";
            ImVec2 buttonSize = ImVec2(120.0f, 30.0f);
            float buttonPosX = (windowSize.x - buttonSize.x) * 0.5f;

            ImGui::SetCursorPosX(buttonPosX);
            if (ImGui::Button(buttonStr, buttonSize)) { std::cout << "Centered button clicked!" << std::endl; }

            ImGui::End();

            EndFrame();
        }
    }

    void EditorApp::CleanupImGui()
    {
        const auto& graphicsContext = m_engine.GetGraphicsContext();
        if (m_CommandPoolObj != VK_NULL_HANDLE)
        {
            vkDestroyCommandPool(graphicsContext.Device, m_CommandPoolObj, nullptr);
            m_CommandPoolObj = VK_NULL_HANDLE;
        }

        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
}

int main()
{
    try
    {
        Editor::EditorApp app;
        app.Run();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Application crashed with critical error: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}
