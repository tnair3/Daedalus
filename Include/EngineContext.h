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
        VkExtent2D SwapchainExtent = {.width = 0, .height = 0};
        VkFence AcquireFence = VK_NULL_HANDLE;
        VkCommandPool CommandPool = VK_NULL_HANDLE;
        VkCommandBuffer CommandBuffer = VK_NULL_HANDLE;
    };

    struct OffscreenRenderTarget {
        VkExtent2D Extent{ .width = 1920, .height = 1080 };
        VkImage Image = VK_NULL_HANDLE;
        VkDeviceMemory ImageMemory = VK_NULL_HANDLE;
        VkImageView ImageView = VK_NULL_HANDLE;
        VkSampler Sampler = VK_NULL_HANDLE;
        VkRenderPass RenderPass = VK_NULL_HANDLE;
        VkFramebuffer Framebuffer = VK_NULL_HANDLE;
    };
}
