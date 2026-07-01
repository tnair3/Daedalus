#pragma once

#include <vulkan/vulkan.h>
#include <vector>

namespace Daedalus {
    struct EngineGraphicsContext
    {
        VkInstance Instance = VK_NULL_HANDLE;
        VkPhysicalDevice PhysicalDevice = VK_NULL_HANDLE;
        VkDevice Device = VK_NULL_HANDLE;
        VkQueue GraphicsQueue = VK_NULL_HANDLE;
        VkDescriptorPool DescriptorPool = VK_NULL_HANDLE;
        VkRenderPass RenderPass = VK_NULL_HANDLE;
        uint32_t QueueFamilyIndex = 0;
        VkSurfaceKHR pSurface = VK_NULL_HANDLE;
        VkSwapchainKHR Swapchain = VK_NULL_HANDLE;
        std::vector<VkImage> SwapchainImages;
        std::vector<VkImageView> SwapchainImageViews;
        std::vector<VkFramebuffer> Framebuffers;
        VkExtent2D SwapchainExtent = {1280, 720};
        VkFence AcquireFence = VK_NULL_HANDLE;
    };
}
