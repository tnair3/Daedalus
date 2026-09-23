#include <Windows.h>
#include <iostream>
#include <tchar.h>
#include <stdexcept>
#include <nfd.hpp>

#include "Main.h"
#include "Window.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

namespace Editor {
    EditorApp::EditorApp() : m_IsRunning(true), m_Window(1280, 720, "Daedalus Engine Editor - v1.0.0")
    {
        m_engine.Initialize(m_Window.GetNativeWindow());
        InitVulkanContext();
        InitImGui();
    }

    EditorApp::~EditorApp()
    {
        CleanupImGui();
        CleanupVulkanContext();
    }

    void EditorApp::InitImGui()
    {
        std::cout << "EditorApp: Initializing ImGui context..." << std::endl;

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForVulkan(m_Window.GetNativeWindow(), true);

        const auto& graphicsContext = m_engine.GetGraphicsContext();

        ImGui_ImplVulkan_InitInfo init_info = {
            .Instance = graphicsContext.Instance,
            .PhysicalDevice = graphicsContext.PhysicalDevice,
            .Device = graphicsContext.Device,
            .QueueFamily = graphicsContext.QueueFamilyIndex,
            .Queue = graphicsContext.GraphicsQueue,
            .DescriptorPool = graphicsContext.DescriptorPool,
            .MinImageCount = 2,
            .ImageCount = 2,
            .PipelineInfoMain = {
                .RenderPass = graphicsContext.RenderPass,
                .Subpass = 0,
                .MSAASamples = VK_SAMPLE_COUNT_1_BIT
            },
            .Allocator = nullptr
        };

        if (!ImGui_ImplVulkan_Init(&init_info))
        {
            throw std::runtime_error("EditorApp Error: Failed to initialize ImGui Vulkan backend!");
        }

        std::cout << "EditorApp: ImGui initialization complete." << std::endl;
    }

    void EditorApp::InitVulkanContext()
    {
        std::cout << "EditorApp: Initializing Editor Vulkan context..." << std::endl;

        const auto& graphicsContext = m_engine.GetGraphicsContext();

        VkCommandPoolCreateInfo poolInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = graphicsContext.QueueFamilyIndex
        };

        if (vkCreateCommandPool(graphicsContext.Device, &poolInfo, nullptr, &m_ImGuiCommandPool) != VK_SUCCESS)
        {
            throw std::runtime_error("EditorApp Error: Failed to create ImGui command pool!");
        }

        VkCommandBufferAllocateInfo allocInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = m_ImGuiCommandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1
        };

        if (vkAllocateCommandBuffers(graphicsContext.Device, &allocInfo, &m_ImGuiCommandBuffer) != VK_SUCCESS)
        {
            throw std::runtime_error("EditorApp Error: Failed to allocate ImGui command buffer!");
        }

        std::cout << "EditorApp: Editor Vulkan context initialized." << std::endl;
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

        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            m_engine.RecreateSwapchain(m_Window.GetNativeWindow());
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            return;
        }

        vkWaitForFences(graphicsContext.Device, 1, &graphicsContext.AcquireFence, VK_TRUE, UINT64_MAX);
        vkResetFences(graphicsContext.Device, 1, &graphicsContext.AcquireFence);

        vkResetCommandBuffer(m_ImGuiCommandBuffer, 0);
        VkCommandBufferBeginInfo beginInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
        };

        vkBeginCommandBuffer(m_ImGuiCommandBuffer, &beginInfo);

        VkClearValue clearColor = { .color = { .float32 = {0.1f, 0.1f, 0.1f, 1.0f} } };

        VkRenderPassBeginInfo rpInfo = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass = graphicsContext.RenderPass,
            .framebuffer = graphicsContext.Framebuffers[imageIndex],
            .renderArea = {
                .offset = { .x = 0, .y = 0 },
                .extent = graphicsContext.SwapchainExtent
            },
            .clearValueCount = 1,
            .pClearValues = &clearColor
        };

        vkCmdBeginRenderPass(m_ImGuiCommandBuffer, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

        ImGui_ImplVulkan_RenderDrawData(draw_data, m_ImGuiCommandBuffer);

        vkCmdEndRenderPass(m_ImGuiCommandBuffer);
        vkEndCommandBuffer(m_ImGuiCommandBuffer);

        VkSubmitInfo submitInfo = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .commandBufferCount = 1,
            .pCommandBuffers = &m_ImGuiCommandBuffer
        };

        vkQueueSubmit(graphicsContext.GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(graphicsContext.GraphicsQueue);

        VkPresentInfoKHR presentInfo = {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .swapchainCount = 1,
            .pSwapchains = &graphicsContext.Swapchain,
            .pImageIndices = &imageIndex
        };
        
        result = vkQueuePresentKHR(graphicsContext.GraphicsQueue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_Window.WasFramebufferResized())
        {
            m_Window.ResetFramebufferResizedFlag();
            m_engine.RecreateSwapchain(m_Window.GetNativeWindow());
        }

        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }
    }

    void EditorApp::Run()
    {
        while (m_IsRunning && !m_Window.ShouldClose())
        {
            m_Window.PollEvents();

            m_engine.Render();

            BeginFrame();

            bool resetLayoutRequested = false;

            static bool showProjectExplorer = true;
            static bool showSceneViewer = true;
            static bool showInspector = true;
            static bool showCodeEditor = true;
            static bool showTerminal = true;
            static bool showVersionControl = true;

            static bool showMetricsWindow = false;
            static bool showDemoWindow = false;

            // Menu bar
            if (ImGui::BeginMainMenuBar())
            {
                if (ImGui::BeginMenu("File"))
                {
                    if (ImGui::MenuItem("New Project", "Ctrl+N")) {  }
                    if (ImGui::MenuItem("Open Project", "Ctrl+O"))
                    {
                        NFD::Guard nfdGuard;
                        nfdu8char_t* outPath = nullptr;
                        nfdu8filteritem_t filterItem[1] = { { .name = "Daedalus Project File", .spec = "myproject" } };
                        nfdresult_t result = NFD::OpenDialog(outPath, filterItem, 1, nullptr);

                        if (result == NFD_OKAY)
                        {
                            std::string selectedPath(outPath);
                            m_engine.LoadProject(selectedPath);
                            NFD::FreePath(outPath);
                        }
                        else if (result == NFD_CANCEL) {  }
                        else
                        {
                            std::cerr << "Error: " << NFD::GetError() << std::endl;
                        }
                    }
                    if (ImGui::BeginMenu("Recent Projects"))
                    {
                        if (ImGui::MenuItem("Test Item 1")) {  }
                        if (ImGui::MenuItem("Test Item 2")) {  }
                        ImGui::EndMenu();
                    }
                    ImGui::Separator();

                    if (ImGui::MenuItem("New Scene", "Ctrl+Shift+N")) {  }
                    if (ImGui::MenuItem("Open Scene", "Ctrl+Shift+O")) {  }
                    if (ImGui::MenuItem("Save Scene", "Ctrl+S")) {  }
                    if (ImGui::MenuItem("Save Scene As", "Ctrl+Shift+S")) {  }
                    if (ImGui::MenuItem("Save All", "Ctrl+Alt+S")) {  }
                    ImGui::Separator();

                    if (ImGui::MenuItem("Build Settings", "Ctrl+Shift+B")) {  }
                    if (ImGui::MenuItem("Export Asset")) {  }
                    ImGui::Separator();

                    if (ImGui::MenuItem("Exit", "Alt+F4")) { m_IsRunning = false; }
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Edit"))
                {
                    if (ImGui::MenuItem("Undo", "Ctrl+Z")) {  }
                    if (ImGui::MenuItem("Redo", "Ctrl+Y")) {  }
                    ImGui::Separator();

                    if (ImGui::MenuItem("Cut", "Ctrl+X")) {  }
                    if (ImGui::MenuItem("Copy", "Ctrl+C")) {  }
                    if (ImGui::MenuItem("Paste", "Ctrl+V")) {  }
                    if (ImGui::MenuItem("Duplicate", "Ctrl+D")) {  }
                    if (ImGui::MenuItem("Delete", "Del")) {  }
                    ImGui::Separator();

                    if (ImGui::MenuItem("Select All", "Ctrl+A")) {  }
                    if (ImGui::MenuItem("Deselect All", "Escape")) {  }
                    ImGui::Separator();

                    if (ImGui::MenuItem("Project Settings")) {  } // Open a dedicated window for input, graphics, audio, physics settings
                    if (ImGui::MenuItem("Editor Preferences")) {  } // Open a dedicated window for theme, keybindings, layout defaults
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("View"))
                {
                    if (ImGui::BeginMenu("Layouts"))
                    {
                        if (ImGui::MenuItem("Default Layout")) { resetLayoutRequested = true; }
                        if (ImGui::MenuItem("Scripting Layout")) {  }
                        ImGui::EndMenu();
                    }
                    if (ImGui::BeginMenu("Panels"))
                    {
                        ImGui::MenuItem("Project Explorer", nullptr, &showProjectExplorer);
                        ImGui::MenuItem("Scene Viewer", nullptr, &showSceneViewer);
                        ImGui::MenuItem("Inspector", nullptr, &showInspector);
                        ImGui::MenuItem("Code Editor", nullptr, &showCodeEditor);
                        ImGui::MenuItem("Terminal / Logs", nullptr, &showTerminal);
                        ImGui::MenuItem("Version Control", nullptr, &showVersionControl);
                        ImGui::EndMenu();
                    }
                    ImGui::Separator();

                    if (ImGui::MenuItem("Toggle Fullscreen", "F11")) {  }
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Entity"))
                {
                    if (ImGui::MenuItem("Create Empty", "Ctrl+Shift+E")) {  }
                    ImGui::Separator();

                    if (ImGui::BeginMenu("2D Sprites"))
                    {
                        if (ImGui::MenuItem("Square")) {  }
                        if (ImGui::MenuItem("Circle")) {  }
                        if (ImGui::MenuItem("Custom Texture")) {  }
                        ImGui::EndMenu();
                    }
                    if (ImGui::BeginMenu("Tilemap"))
                    {
                        if (ImGui::MenuItem("Rectangular Tilemap")) {  }
                        if (ImGui::MenuItem("Isometric Tilemap")) {  }
                        ImGui::EndMenu();
                    }
                    if (ImGui::BeginMenu("Lights"))
                    {
                        if (ImGui::MenuItem("Point Light")) {  }
                        if (ImGui::MenuItem("Global / Ambient Light")) {  }
                        if (ImGui::MenuItem("Spotlight")) {  }
                        ImGui::EndMenu();
                    }
                    if (ImGui::MenuItem("2D Camera")) {  }
                    if (ImGui::MenuItem("Audio Source")) {  }
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Component"))
                {
                    if (ImGui::BeginMenu("Rendering"))
                    {
                        if (ImGui::MenuItem("Sprite Renderer")) {  }
                        if (ImGui::MenuItem("Tilemap Renderer")) {  }
                        if (ImGui::MenuItem("Sorting Group")) {  }
                        ImGui::EndMenu();
                    }
                    if (ImGui::BeginMenu("2D Physics"))
                    {
                        if (ImGui::MenuItem("RigidBody 2D")) {  }
                        if (ImGui::MenuItem("Box Collider 2D")) {  }
                        if (ImGui::MenuItem("Circle Collider 2D")) {  }
                        if (ImGui::MenuItem("Polygon Collider 2D")) {  }
                        ImGui::EndMenu();
                    }
                    if (ImGui::BeginMenu("Animation"))
                    {
                        if (ImGui::MenuItem("Animator 2D")) {  }
                        if (ImGui::MenuItem("Sprite Sheet Player")) {  }
                        ImGui::EndMenu();
                    }
                    if (ImGui::BeginMenu("Audio"))
                    {
                        if (ImGui::MenuItem("Audio Source 2D")) {  }
                        if (ImGui::MenuItem("Audio Listener")) {  }
                        ImGui::EndMenu();
                    }
                    if (ImGui::BeginMenu("Scripts"))
                    {
                        if (ImGui::MenuItem("Attach Script")) {  }
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Tools"))
                {
                    if (ImGui::MenuItem("Tile Palette Editor")) {  }
                    if (ImGui::MenuItem("Sprite Editor")) {  }
                    ImGui::Separator();

                    if (ImGui::MenuItem("Recompile Scripts / Shaders", "F5")) {  }
                    ImGui::MenuItem("ImGui Metrics", nullptr, &showMetricsWindow);
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Help"))
                {
                    if (ImGui::MenuItem("Engine Documentation")) {  }
                    ImGui::MenuItem("ImGui Demo Window", nullptr, &showDemoWindow);
                    ImGui::Separator();

                    if (ImGui::MenuItem("About Daedalus Engine")) {  }
                    ImGui::EndMenu();
                }

                ImGui::EndMainMenuBar();
            }

            // Dockspace
            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize);
            ImGui::SetNextWindowViewport(viewport->ID);

            ImGuiWindowFlags hostWindowFlags =
                        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
                        ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoBackground;

            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

            ImGui::Begin("EditorRootDockSpaceWindow", nullptr, hostWindowFlags);
            ImGui::PopStyleVar(3);

            ImGuiID dockSpace_id = ImGui::GetID("EditorDockSpace");

            if (ImGui::DockBuilderGetNode(dockSpace_id) == nullptr || resetLayoutRequested)
            {
                showProjectExplorer = true;
                showSceneViewer = true;
                showInspector = true;
                showCodeEditor = true;
                showTerminal = true;
                showVersionControl = true;

                showMetricsWindow = false;
                showDemoWindow = false;

                ImGui::DockBuilderRemoveNode(dockSpace_id);
                ImGui::DockBuilderAddNode(dockSpace_id, ImGuiDockNodeFlags_DockSpace);
                ImGui::DockBuilderSetNodeSize(dockSpace_id, viewport->WorkSize);

                ImGuiID dockIdCenter;
                ImGuiID dockIdLeft = ImGui::DockBuilderSplitNode(dockSpace_id, ImGuiDir_Left, 0.20f, nullptr, &dockIdCenter);
                ImGuiID dockIdRight = ImGui::DockBuilderSplitNode(dockIdCenter, ImGuiDir_Right, 0.25f, nullptr, &dockIdCenter);

                const float centerColumnWidth = viewport->WorkSize.x * 0.60f;
                const float targetSceneHeight = centerColumnWidth * (9.0f / 16.0f);
                float centerTopHeightRatio = targetSceneHeight / viewport->WorkSize.y;
                centerTopHeightRatio = ImClamp(centerTopHeightRatio, 0.20f, 0.85f);

                ImGuiID dockIdLeftBottom;
                ImGuiID dockIdLeftTop = ImGui::DockBuilderSplitNode(dockIdLeft, ImGuiDir_Up, 0.60f, nullptr, &dockIdLeftBottom);

                ImGuiID dockIdCenterBottom;
                ImGuiID dockIdCenterTop = ImGui::DockBuilderSplitNode(dockIdCenter, ImGuiDir_Up, centerTopHeightRatio, nullptr, &dockIdCenterBottom);

                ImGui::DockBuilderDockWindow("Project Explorer", dockIdLeftTop);
                ImGui::DockBuilderDockWindow("Version Control", dockIdLeftBottom);
                ImGui::DockBuilderDockWindow("Scene Viewer", dockIdCenterTop);
                ImGui::DockBuilderDockWindow("Code Editor", dockIdCenterTop);
                ImGui::DockBuilderDockWindow("Terminal Output", dockIdCenterBottom);
                ImGui::DockBuilderDockWindow("Inspector", dockIdRight);

                ImGui::DockBuilderFinish(dockSpace_id);
            }

            ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_NoWindowMenuButton;

            ImGui::DockSpace(dockSpace_id, ImVec2(0.0f, 0.0f), dockspaceFlags);
            ImGui::End();

            if (showProjectExplorer)
            {
                ImGui::Begin("Project Explorer");
                ImGui::Text("File hierarchy tree goes here.");
                ImGui::End();
            }

            if (showVersionControl)
            {
                ImGui::Begin("Version Control");
                ImGui::Text("Git history logs.");
                ImGui::End();
            }

            if (showSceneViewer)
            {
                ImGui::Begin("Scene Viewer");
                if (const auto& offscreenTarget = m_engine.GetRenderTarget(); offscreenTarget.ImageView != VK_NULL_HANDLE && offscreenTarget.Sampler != VK_NULL_HANDLE)
                {
                    if (m_CurrentViewportImageView != offscreenTarget.ImageView)
                    {
                        if (m_ViewportTextureID != 0) { ImGui_ImplVulkan_RemoveTexture(reinterpret_cast<VkDescriptorSet>(m_ViewportTextureID)); }

                        m_ViewportTextureID = reinterpret_cast<ImTextureID>(ImGui_ImplVulkan_AddTexture(
                            offscreenTarget.Sampler,
                            offscreenTarget.ImageView,
                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                        ));

                        m_CurrentViewportImageView = offscreenTarget.ImageView;
                    }

                    ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
                    ImGui::Image(m_ViewportTextureID, viewportPanelSize);
                }
                else
                {
                    ImGui::Text("Vulkan Viewport Render Target Window (Error in displaying scene)");
                }
                ImGui::End();
            }

            if (showCodeEditor)
            {
                ImGui::Begin("Code Editor");
                ImGui::Text("Built in code editor window");
                ImGui::End();
            }

            if (showTerminal)
            {
                ImGui::Begin("Terminal Output");
                ImGui::Text("Build output logs go here.");
                ImGui::End();
            }

            if (showInspector)
            {
                ImGui::Begin("Inspector");
                ImGui::Text("Component properties.");
                ImGui::End();
            }

            if (showMetricsWindow) { ImGui::ShowMetricsWindow(&showMetricsWindow); }
            if (showDemoWindow) { ImGui::ShowDemoWindow(&showDemoWindow); }

            EndFrame();
        }
    }

    void EditorApp::CleanupImGui()
    {
        std::cout << "EditorApp: Cleaning up ImGui..." << std::endl;

        if (m_ViewportTextureID != 0)
        {
            ImGui_ImplVulkan_RemoveTexture(reinterpret_cast<VkDescriptorSet>(m_ViewportTextureID));
            m_ViewportTextureID = 0;
            m_CurrentViewportImageView = VK_NULL_HANDLE;
        }

        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyPlatformWindows();
        ImGui::DestroyContext();

        std::cout << "EditorApp: ImGui cleanup complete." << std::endl;
    }

    void EditorApp::CleanupVulkanContext()
    {
        std::cout << "EditorApp: Cleaning up Editor Vulkan context..." << std::endl;

        const auto& graphicsContext = m_engine.GetGraphicsContext();

        if (graphicsContext.Device != VK_NULL_HANDLE)
        {
            std::cout << "EditorApp: Awaiting device idle..." << std::endl;
            vkDeviceWaitIdle(graphicsContext.Device);
        }

        if (m_ImGuiCommandPool != VK_NULL_HANDLE)
        {
            vkDestroyCommandPool(graphicsContext.Device, m_ImGuiCommandPool, nullptr);
            m_ImGuiCommandPool = VK_NULL_HANDLE;
            m_ImGuiCommandBuffer = VK_NULL_HANDLE;
        }

        std::cout << "EditorApp: Editor Vulkan context cleanup complete." << std::endl;
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
