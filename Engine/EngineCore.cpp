#include <vulkan/vulkan.h>
#include <iostream>
#include <fstream>
#include <vector>

#include "EngineCore.h"
#include "Graphics/Pipelines/TrianglePipeline.h"

namespace Daedalus {
    EngineCore::EngineCore() = default;

    EngineCore::~EngineCore() { Shutdown(); }

    void EngineCore::Initialize(GLFWwindow* window)
    {
        std::cout << "EngineCore: Initializing Vulkan context..." << std::endl;

        m_OffscreenTarget.Extent = { .width = 1920, .height = 1080 };
        m_TrianglePipeline = std::make_unique<TrianglePipeline>();

        // Instance Creation
        VkApplicationInfo appInfo = {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pApplicationName = "Daedalus",
            .apiVersion = VK_API_VERSION_1_3
        };

        VkInstanceCreateInfo instanceInfo = {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pApplicationInfo = &appInfo,
        };

        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        instanceInfo.enabledExtensionCount = glfwExtensionCount;
        instanceInfo.ppEnabledExtensionNames = glfwExtensions;

        const char* validationLayers[] = {"VK_LAYER_KHRONOS_validation"};
        instanceInfo.enabledLayerCount = 1;
        instanceInfo.ppEnabledLayerNames = validationLayers;

        vkCreateInstance(&instanceInfo, nullptr, &m_GraphicsContext.Instance);

        if (glfwCreateWindowSurface(m_GraphicsContext.Instance, window, nullptr, &m_GraphicsContext.pSurface) != VK_SUCCESS)
        {
            throw std::runtime_error("Vulkan Bootstrap Error: Failed to create window surface.");
        }

        // Physical and Logical Device Selection
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(m_GraphicsContext.Instance, &deviceCount, nullptr);
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(m_GraphicsContext.Instance, &deviceCount, devices.data());
        m_GraphicsContext.PhysicalDevice = devices[0];

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(m_GraphicsContext.PhysicalDevice, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(m_GraphicsContext.PhysicalDevice, &queueFamilyCount, queueFamilies.data());

        int graphicsFamilyIndex = -1;
        for (uint32_t i = 0; i < queueFamilyCount; i++)
        {
            if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                graphicsFamilyIndex = i;
                break;
            }
        }
        m_GraphicsContext.QueueFamilyIndex = graphicsFamilyIndex;

        float queuePriority = 1.0f;
        VkDeviceQueueCreateInfo deviceQueueInfo = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = m_GraphicsContext.QueueFamilyIndex,
            .queueCount = 1,
            .pQueuePriorities = &queuePriority
        };

        // Enable KHR Swapchain extension explicitly
        const char* deviceExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

        VkDeviceCreateInfo deviceInfo = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .queueCreateInfoCount = 1,
            .pQueueCreateInfos = &deviceQueueInfo,
            .enabledExtensionCount = 1,
            .ppEnabledExtensionNames = deviceExtensions
        };

        if (vkCreateDevice(m_GraphicsContext.PhysicalDevice, &deviceInfo, nullptr, &m_GraphicsContext.Device) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create Vulkan logical device!");
        }

        vkGetDeviceQueue(m_GraphicsContext.Device, m_GraphicsContext.QueueFamilyIndex, 0, &m_GraphicsContext.GraphicsQueue);

        // Offscreen Target Image, Memory, and View
        VkImageCreateInfo imageInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = VK_FORMAT_R8G8B8A8_UNORM,
            .extent = { .width = 1920, .height = 1080, .depth = 1 },
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        };

        if (vkCreateImage(m_GraphicsContext.Device, &imageInfo, nullptr, &m_OffscreenTarget.Image) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create offscreen image!");
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(m_GraphicsContext.Device, m_OffscreenTarget.Image, &memRequirements);

        VkMemoryAllocateInfo memAllocateInfo = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = memRequirements.size,
            .memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
        };

        if (vkAllocateMemory(m_GraphicsContext.Device, &memAllocateInfo, nullptr, &m_OffscreenTarget.ImageMemory) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to allocate offscreen image memory!");
        }

        vkBindImageMemory(m_GraphicsContext.Device, m_OffscreenTarget.Image, m_OffscreenTarget.ImageMemory, 0);

        VkImageViewCreateInfo imageViewInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = m_OffscreenTarget.Image,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = VK_FORMAT_R8G8B8A8_UNORM,
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        };

        if (vkCreateImageView(m_GraphicsContext.Device, &imageViewInfo, nullptr, &m_OffscreenTarget.ImageView) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create offscreen image view!");
        }

        // Offscreen Render Pass Creation
        VkAttachmentDescription offscreenColorAttachment = {
            .format = VK_FORMAT_R8G8B8A8_UNORM,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        };

        VkAttachmentReference offscreenColorRef = {
            .attachment = 0,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        };

        VkSubpassDescription offscreenSubpass = {
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .colorAttachmentCount = 1,
            .pColorAttachments = &offscreenColorRef,
        };

        // Subpass dependency to sync color attachment writes with fragment shader reads
        VkSubpassDependency offscreenSubpassDependency = {
            .srcSubpass = VK_SUBPASS_EXTERNAL,
            .dstSubpass = 0,
            .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .dstAccessMask = VK_ACCESS_SHADER_READ_BIT
        };

        VkRenderPassCreateInfo offscreenPassInfo = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .attachmentCount = 1,
            .pAttachments = &offscreenColorAttachment,
            .subpassCount = 1,
            .pSubpasses = &offscreenSubpass,
            .dependencyCount = 1,
            .pDependencies = &offscreenSubpassDependency
        };

        if (vkCreateRenderPass(m_GraphicsContext.Device, &offscreenPassInfo, nullptr, &m_OffscreenTarget.RenderPass) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create offscreen render pass!");
        }

        // Offscreen Framebuffer Creation
        VkFramebufferCreateInfo offscreenFbInfo = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = m_OffscreenTarget.RenderPass,
            .attachmentCount = 1,
            .pAttachments = &m_OffscreenTarget.ImageView,
            .width = m_OffscreenTarget.Extent.width,
            .height = m_OffscreenTarget.Extent.height,
            .layers = 1
        };

        if (vkCreateFramebuffer(m_GraphicsContext.Device, &offscreenFbInfo, nullptr, &m_OffscreenTarget.Framebuffer) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create offscreen framebuffer!");
        }

        VkSamplerCreateInfo samplerInfo = {
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .magFilter = VK_FILTER_LINEAR,
            .minFilter = VK_FILTER_LINEAR,
            .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
            .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            .mipLodBias = 0.0f,
            .anisotropyEnable = VK_FALSE,
            .maxAnisotropy = 1.0f,
            .compareEnable = VK_FALSE,
            .compareOp = VK_COMPARE_OP_ALWAYS,
            .minLod = 0.0f,
            .maxLod = 1.0f,
            .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
            .unnormalizedCoordinates = VK_FALSE,
        };

        if (vkCreateSampler(m_GraphicsContext.Device, &samplerInfo, nullptr, &m_OffscreenTarget.Sampler) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create offscreen texture sampler!");
        }

        // Create Descriptor Pool
        VkDescriptorPoolSize descriptorPoolSize[] = {
            {.type = VK_DESCRIPTOR_TYPE_SAMPLER, .descriptorCount = 1000},
            {.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 1000},
            {.type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, .descriptorCount = 1000},
            {.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, .descriptorCount = 1000},
            {.type = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, .descriptorCount = 1000},
            {.type = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, .descriptorCount = 1000},
            {.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 1000},
            {.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, .descriptorCount = 1000},
            {.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, .descriptorCount = 1000},
            {.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, .descriptorCount = 1000},
            {.type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, .descriptorCount = 1000}
        };

        VkDescriptorPoolCreateInfo descriptorPoolInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
            .poolSizeCount = static_cast<uint32_t>(std::size(descriptorPoolSize)),
            .pPoolSizes = descriptorPoolSize
        };
        descriptorPoolInfo.maxSets = 1000 * descriptorPoolInfo.poolSizeCount;
        vkCreateDescriptorPool(m_GraphicsContext.Device, &descriptorPoolInfo, nullptr, &m_GraphicsContext.DescriptorPool);

        // Setup Render Pass targeting proper window presentation layout
        VkAttachmentDescription colorAttachment = {
            .format = VK_FORMAT_B8G8R8A8_UNORM,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        };

        VkAttachmentReference colorAttachmentRef = {
            .attachment = 0,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        };

        VkSubpassDescription subpass = {
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .colorAttachmentCount = 1,
            .pColorAttachments = &colorAttachmentRef
        };

        VkRenderPassCreateInfo renderPassInfo = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .attachmentCount = 1,
            .pAttachments = &colorAttachment,
            .subpassCount = 1,
            .pSubpasses = &subpass
        };
        vkCreateRenderPass(m_GraphicsContext.Device, &renderPassInfo, nullptr, &m_GraphicsContext.RenderPass);

        VkSurfaceCapabilitiesKHR capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_GraphicsContext.PhysicalDevice, m_GraphicsContext.pSurface, &capabilities);

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        m_GraphicsContext.SwapchainExtent = { .width = static_cast<uint32_t>(width), .height = static_cast<uint32_t>(height) };

        // Create Engine Command Pool
        VkCommandPoolCreateInfo commandPoolInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = m_GraphicsContext.QueueFamilyIndex
        };

        if (vkCreateCommandPool(m_GraphicsContext.Device, &commandPoolInfo, nullptr, &m_GraphicsContext.CommandPool) != VK_SUCCESS)
        {
            throw std::runtime_error("EngineCore: Failed to create engine command pool!");
        }

        // Allocate Engine Command Buffer
        VkCommandBufferAllocateInfo commandBufferAllocInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = m_GraphicsContext.CommandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1
        };

        if (vkAllocateCommandBuffers(m_GraphicsContext.Device, &commandBufferAllocInfo, &m_GraphicsContext.CommandBuffer) != VK_SUCCESS)
        {
            throw std::runtime_error("EngineCore: Failed to allocate engine command buffer!");
        }

        // Perform initial image layout transition
        TransitionImageLayout(
            m_OffscreenTarget.Image,
            VK_FORMAT_R8G8B8A8_UNORM,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        );

        // Create Swapchain
        VkSwapchainCreateInfoKHR swapchainInfo = {
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface = m_GraphicsContext.pSurface,
            .minImageCount = 2,
            .imageFormat = VK_FORMAT_B8G8R8A8_UNORM,
            .imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
            .imageExtent = m_GraphicsContext.SwapchainExtent,
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = VK_PRESENT_MODE_FIFO_KHR,
            .clipped = VK_TRUE
        };

        vkCreateSwapchainKHR(m_GraphicsContext.Device, &swapchainInfo, nullptr, &m_GraphicsContext.Swapchain);

        uint32_t imageCount = 0;
        vkGetSwapchainImagesKHR(m_GraphicsContext.Device, m_GraphicsContext.Swapchain, &imageCount, nullptr);
        m_GraphicsContext.SwapchainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(m_GraphicsContext.Device, m_GraphicsContext.Swapchain, &imageCount, m_GraphicsContext.SwapchainImages.data());

        // Build Image Views and Framebuffers per Swapchain Image
        m_GraphicsContext.SwapchainImageViews.resize(imageCount);
        m_GraphicsContext.Framebuffers.resize(imageCount);

        for (size_t i = 0; i < imageCount; i++)
        {
            VkImageViewCreateInfo viewInfo = {
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .image = m_GraphicsContext.SwapchainImages[i],
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = VK_FORMAT_B8G8R8A8_UNORM,
                .subresourceRange = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1
                }
            };

            vkCreateImageView(m_GraphicsContext.Device, &viewInfo, nullptr, &m_GraphicsContext.SwapchainImageViews[i]);

            VkFramebufferCreateInfo framebufferInfo = {
                .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                .renderPass = m_GraphicsContext.RenderPass,
                .attachmentCount = 1,
                .pAttachments = &m_GraphicsContext.SwapchainImageViews[i],
                .width = m_GraphicsContext.SwapchainExtent.width,
                .height = m_GraphicsContext.SwapchainExtent.height,
                .layers = 1
            };

            vkCreateFramebuffer(m_GraphicsContext.Device, &framebufferInfo, nullptr, &m_GraphicsContext.Framebuffers[i]);
        }

        // Create synchronization fence for image acquisition
        VkFenceCreateInfo fenceInfo = {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = 0
        };

        if (vkCreateFence(m_GraphicsContext.Device, &fenceInfo, nullptr, &m_GraphicsContext.AcquireFence) != VK_SUCCESS)
        {
            throw std::runtime_error("Vulkan Bootstrap Error: Failed to create acquire fence.");
        }

        m_TrianglePipeline->Initialize(m_GraphicsContext.Device, m_OffscreenTarget.RenderPass, m_OffscreenTarget.Extent);

        std::cout << "EngineCore: Vulkan bootstrap initialized." << std::endl;
    }

    void EngineCore::Update() {}

    void EngineCore::Render()
    {
        vkResetCommandBuffer(m_GraphicsContext.CommandBuffer, 0);

        VkCommandBufferBeginInfo beginInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
        };

        if (vkBeginCommandBuffer(m_GraphicsContext.CommandBuffer, &beginInfo) != VK_SUCCESS) { return; }

        VkClearValue clearColor = { .color = { .float32 = { 0.1f, 0.15f, 0.2f, 1.0f } } };

        VkRenderPassBeginInfo renderPassInfo = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass = m_OffscreenTarget.RenderPass,
            .framebuffer = m_OffscreenTarget.Framebuffer,
            .renderArea = {
                .offset = { .x = 0, .y = 0 },
                .extent = m_OffscreenTarget.Extent
            },
            .clearValueCount = 1,
            .pClearValues = &clearColor
        };

        vkCmdBeginRenderPass(m_GraphicsContext.CommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        // =================================
        // Geometry draw commands go below
        // =================================

        m_TrianglePipeline->RecordDraw(m_GraphicsContext.CommandBuffer, m_OffscreenTarget.Extent);

        VkViewport viewport = {
            .x = 0.0f,
            .y = 0.0f,
            .width = static_cast<float>(m_OffscreenTarget.Extent.width),
            .height = static_cast<float>(m_OffscreenTarget.Extent.height),
            .minDepth = 0.0f,
            .maxDepth = 1.0f
        };
        vkCmdSetViewport(m_GraphicsContext.CommandBuffer, 0, 1, &viewport);

        VkRect2D scissor = {
            .offset = { .x = 0, .y = 0 },
            .extent = m_OffscreenTarget.Extent
        };
        vkCmdSetScissor(m_GraphicsContext.CommandBuffer, 0, 1, &scissor);

        vkCmdDraw(m_GraphicsContext.CommandBuffer, 3, 1, 0, 0);

        // =================================
        // Geometry draw commands go above
        // =================================

        vkCmdEndRenderPass(m_GraphicsContext.CommandBuffer);

        if (vkEndCommandBuffer(m_GraphicsContext.CommandBuffer) != VK_SUCCESS) { return; }

        VkSubmitInfo submitInfo = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .commandBufferCount = 1,
            .pCommandBuffers = &m_GraphicsContext.CommandBuffer
        };

        vkQueueSubmit(m_GraphicsContext.GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(m_GraphicsContext.GraphicsQueue);
    }

    void EngineCore::Shutdown()
    {
        std::cout << "EngineCore: Shutting down..." << std::endl;

        if (m_GraphicsContext.Device != VK_NULL_HANDLE)
        {
            std::cout << "EngineCore: Awaiting device idle..." << std::endl;
            vkDeviceWaitIdle(m_GraphicsContext.Device);
        }

        m_TrianglePipeline->Cleanup(m_GraphicsContext.Device);

        if (m_OffscreenTarget.Sampler) vkDestroySampler(m_GraphicsContext.Device, m_OffscreenTarget.Sampler, nullptr);
        if (m_OffscreenTarget.Framebuffer) vkDestroyFramebuffer(m_GraphicsContext.Device, m_OffscreenTarget.Framebuffer, nullptr);
        if (m_OffscreenTarget.RenderPass) vkDestroyRenderPass(m_GraphicsContext.Device, m_OffscreenTarget.RenderPass, nullptr);
        if (m_OffscreenTarget.ImageView) vkDestroyImageView(m_GraphicsContext.Device, m_OffscreenTarget.ImageView, nullptr);
        if (m_OffscreenTarget.Image) vkDestroyImage(m_GraphicsContext.Device, m_OffscreenTarget.Image, nullptr);
        if (m_OffscreenTarget.ImageMemory) vkFreeMemory(m_GraphicsContext.Device, m_OffscreenTarget.ImageMemory, nullptr);

        for (auto framebuffer : m_GraphicsContext.Framebuffers) { if (framebuffer) vkDestroyFramebuffer(m_GraphicsContext.Device, framebuffer, nullptr); }
        for (auto imageView : m_GraphicsContext.SwapchainImageViews) { if (imageView) vkDestroyImageView(m_GraphicsContext.Device, imageView, nullptr); }

        m_GraphicsContext.Framebuffers.clear();
        m_GraphicsContext.SwapchainImageViews.clear();
        m_GraphicsContext.SwapchainImages.clear();

        if (m_GraphicsContext.AcquireFence) vkDestroyFence(m_GraphicsContext.Device, m_GraphicsContext.AcquireFence, nullptr);
        if (m_GraphicsContext.Swapchain) vkDestroySwapchainKHR(m_GraphicsContext.Device, m_GraphicsContext.Swapchain, nullptr);
        if (m_GraphicsContext.RenderPass) vkDestroyRenderPass(m_GraphicsContext.Device, m_GraphicsContext.RenderPass, nullptr);
        if (m_GraphicsContext.DescriptorPool) vkDestroyDescriptorPool(m_GraphicsContext.Device, m_GraphicsContext.DescriptorPool, nullptr);
        if (m_GraphicsContext.CommandPool) vkDestroyCommandPool(m_GraphicsContext.Device, m_GraphicsContext.CommandPool, nullptr);
        if (m_GraphicsContext.pSurface) vkDestroySurfaceKHR(m_GraphicsContext.Instance, m_GraphicsContext.pSurface, nullptr);
        if (m_GraphicsContext.Device) vkDestroyDevice(m_GraphicsContext.Device, nullptr);
        if (m_GraphicsContext.Instance) vkDestroyInstance(m_GraphicsContext.Instance, nullptr);

        m_OffscreenTarget = {};
        m_GraphicsContext = {};

        std::cout << "EngineCore: Shutdown complete." << std::endl;
    }

    void EngineCore::RecreateSwapchain(GLFWwindow* window)
    {
        int width = 0, height = 0;
        glfwGetFramebufferSize(window, &width, &height);

        while (width == 0 || height == 0)
        {
            glfwGetFramebufferSize(window, &width, &height);
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(m_GraphicsContext.Device);

        for (auto framebuffer : m_GraphicsContext.Framebuffers)
            if (framebuffer) vkDestroyFramebuffer(m_GraphicsContext.Device, framebuffer, nullptr);
        for (auto imageView : m_GraphicsContext.SwapchainImageViews)
            if (imageView) vkDestroyImageView(m_GraphicsContext.Device, imageView, nullptr);

        m_GraphicsContext.Framebuffers.clear();
        m_GraphicsContext.SwapchainImageViews.clear();

        VkSwapchainKHR oldSwapchain = m_GraphicsContext.Swapchain;

        VkSurfaceCapabilitiesKHR capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_GraphicsContext.PhysicalDevice, m_GraphicsContext.pSurface, &capabilities);
        m_GraphicsContext.SwapchainExtent = { .width = static_cast<uint32_t>(width), .height = static_cast<uint32_t>(height) };

        VkSwapchainCreateInfoKHR swapchainInfo = {
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface = m_GraphicsContext.pSurface,
            .minImageCount = 2,
            .imageFormat = VK_FORMAT_B8G8R8A8_UNORM,
            .imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
            .imageExtent = m_GraphicsContext.SwapchainExtent,
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = VK_PRESENT_MODE_FIFO_KHR,
            .clipped = VK_TRUE,
            .oldSwapchain = oldSwapchain
        };

        vkCreateSwapchainKHR(m_GraphicsContext.Device, &swapchainInfo, nullptr, &m_GraphicsContext.Swapchain);

        if (oldSwapchain != VK_NULL_HANDLE) { vkDestroySwapchainKHR(m_GraphicsContext.Device, oldSwapchain, nullptr); }

        uint32_t imageCount = 0;
        vkGetSwapchainImagesKHR(m_GraphicsContext.Device, m_GraphicsContext.Swapchain, &imageCount, nullptr);
        m_GraphicsContext.SwapchainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(m_GraphicsContext.Device, m_GraphicsContext.Swapchain, &imageCount, m_GraphicsContext.SwapchainImages.data());

        m_GraphicsContext.SwapchainImageViews.resize(imageCount);
        m_GraphicsContext.Framebuffers.resize(imageCount);

        for (size_t i = 0; i < imageCount; i++)
        {
            VkImageViewCreateInfo viewInfo = {
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .image = m_GraphicsContext.SwapchainImages[i],
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = VK_FORMAT_B8G8R8A8_UNORM,
                .subresourceRange = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1
                }
            };

            vkCreateImageView(m_GraphicsContext.Device, &viewInfo, nullptr, &m_GraphicsContext.SwapchainImageViews[i]);

            VkFramebufferCreateInfo framebufferInfo = {
                .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                .renderPass = m_GraphicsContext.RenderPass,
                .attachmentCount = 1,
                .pAttachments = &m_GraphicsContext.SwapchainImageViews[i],
                .width = m_GraphicsContext.SwapchainExtent.width,
                .height = m_GraphicsContext.SwapchainExtent.height,
                .layers = 1
            };

            vkCreateFramebuffer(m_GraphicsContext.Device, &framebufferInfo, nullptr, &m_GraphicsContext.Framebuffers[i]);
        }
    }

    bool EngineCore::LoadProject(const std::string& projectPath)
    {
        std::cout << "Testing: Opening project from: " << projectPath << std::endl;
        m_ActiveProjectPath = projectPath;

        return true;
    }

    uint32_t EngineCore::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const
    {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(m_GraphicsContext.PhysicalDevice, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
        {
            if ((typeFilter & (1 << i)) &&
               (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                return i;
            }
        }

        throw std::runtime_error("Failed to find suitable GPU memory type!");
    }

    void EngineCore::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
    {
        VkCommandBufferAllocateInfo allocInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = m_GraphicsContext.CommandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1
        };

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(m_GraphicsContext.Device, &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        };
        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        VkImageMemoryBarrier barrier = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .oldLayout = oldLayout,
            .newLayout = newLayout,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = image,
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        };

        VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        }

        vkCmdPipelineBarrier(
            commandBuffer,
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );

        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .commandBufferCount = 1,
            .pCommandBuffers = &commandBuffer
        };

        vkQueueSubmit(m_GraphicsContext.GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(m_GraphicsContext.GraphicsQueue);

        vkFreeCommandBuffers(m_GraphicsContext.Device, m_GraphicsContext.CommandPool, 1, &commandBuffer);
    }
}
