#include <vulkan/vulkan.h>
#include <iostream>
#include <vector>

#include "EngineCore.h"

namespace Daedalus {
    EngineCore::EngineCore() {}

    EngineCore::~EngineCore() { Shutdown(); }

    void EngineCore::Initialize(GLFWwindow* window)
    {
        std::cout << "EditorVulkanBootstrap: Initializing minimal Vulkan context for UI testing..." << std::endl;

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Daedalus Project Editor";
        appInfo.apiVersion = VK_API_VERSION_1_3;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        createInfo.enabledExtensionCount = glfwExtensionCount;
        createInfo.ppEnabledExtensionNames = glfwExtensions;

        const char* validationLayers[] = {"VK_LAYER_KHRONOS_validation"};
        createInfo.enabledLayerCount = 1;
        createInfo.ppEnabledLayerNames = validationLayers;

        vkCreateInstance(&createInfo, nullptr, &m_GraphicsContext.Instance);

        if (glfwCreateWindowSurface(m_GraphicsContext.Instance, window, nullptr, &m_GraphicsContext.pSurface) != VK_SUCCESS) { throw std::runtime_error("Vulkan Bootstrap Error: Failed to create window surface."); }

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
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = m_GraphicsContext.QueueFamilyIndex;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        // Enable KHR Swapchain extension explicitly
        const char* deviceExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

        VkDeviceCreateInfo deviceCreateInfo{};
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.queueCreateInfoCount = 1;
        deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
        deviceCreateInfo.enabledExtensionCount = 1;
        deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions;

        vkCreateDevice(m_GraphicsContext.PhysicalDevice, &deviceCreateInfo, nullptr, &m_GraphicsContext.Device);
        vkGetDeviceQueue(m_GraphicsContext.Device, m_GraphicsContext.QueueFamilyIndex, 0, &m_GraphicsContext.GraphicsQueue);

        // Create Descriptor Pool
        VkDescriptorPoolSize poolSizes[] = {
            {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
            {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
            {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
            {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}
        };
        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        poolInfo.poolSizeCount = static_cast<uint32_t>(std::size(poolSizes));
        poolInfo.pPoolSizes = poolSizes;
        poolInfo.maxSets = 1000 * poolInfo.poolSizeCount;
        vkCreateDescriptorPool(m_GraphicsContext.Device, &poolInfo, nullptr, &m_GraphicsContext.DescriptorPool);

        // Setup Render Pass targeting proper window presentation layout
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = VK_FORMAT_B8G8R8A8_UNORM;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        vkCreateRenderPass(m_GraphicsContext.Device, &renderPassInfo, nullptr, &m_GraphicsContext.RenderPass);

        VkSurfaceCapabilitiesKHR capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_GraphicsContext.PhysicalDevice, m_GraphicsContext.pSurface, &capabilities);

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        m_GraphicsContext.SwapchainExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

        // Create Swapchain
        VkSwapchainCreateInfoKHR swapchainInfo{};
        swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapchainInfo.surface = m_GraphicsContext.pSurface;
        swapchainInfo.minImageCount = 2;
        swapchainInfo.imageFormat = VK_FORMAT_B8G8R8A8_UNORM;
        swapchainInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        swapchainInfo.imageExtent = m_GraphicsContext.SwapchainExtent;
        swapchainInfo.imageArrayLayers = 1;
        swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchainInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapchainInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
        swapchainInfo.clipped = VK_TRUE;

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
            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = m_GraphicsContext.SwapchainImages[i];
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = VK_FORMAT_B8G8R8A8_UNORM;
            viewInfo.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

            vkCreateImageView(m_GraphicsContext.Device, &viewInfo, nullptr, &m_GraphicsContext.SwapchainImageViews[i]);

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = m_GraphicsContext.RenderPass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = &m_GraphicsContext.SwapchainImageViews[i];
            framebufferInfo.width = m_GraphicsContext.SwapchainExtent.width;
            framebufferInfo.height = m_GraphicsContext.SwapchainExtent.height;
            framebufferInfo.layers = 1;

            vkCreateFramebuffer(m_GraphicsContext.Device, &framebufferInfo, nullptr, &m_GraphicsContext.Framebuffers[i]);
        }

        // Create synchronization fence for image acquisition
        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = 0;

        if (vkCreateFence(m_GraphicsContext.Device, &fenceInfo, nullptr, &m_GraphicsContext.AcquireFence) != VK_SUCCESS) { throw std::runtime_error("Vulkan Bootstrap Error: Failed to create acquire fence."); }

        std::cout << "EditorVulkanBootstrap: Minimal vulkan bootstrap initialised" << std::endl;
    }

    void EngineCore::Update() {}

    void EngineCore::Shutdown()
    {
        for (auto framebuffer : m_GraphicsContext.Framebuffers) { if (framebuffer) vkDestroyFramebuffer(m_GraphicsContext.Device, framebuffer, nullptr); }
        for (auto imageView : m_GraphicsContext.SwapchainImageViews) { if (imageView) vkDestroyImageView(m_GraphicsContext.Device, imageView, nullptr); }

        if (m_GraphicsContext.AcquireFence) vkDestroyFence(m_GraphicsContext.Device, m_GraphicsContext.AcquireFence, nullptr);
        if (m_GraphicsContext.Swapchain) vkDestroySwapchainKHR(m_GraphicsContext.Device, m_GraphicsContext.Swapchain, nullptr);
        if (m_GraphicsContext.RenderPass) vkDestroyRenderPass(m_GraphicsContext.Device, m_GraphicsContext.RenderPass, nullptr);
        if (m_GraphicsContext.DescriptorPool) vkDestroyDescriptorPool(m_GraphicsContext.Device, m_GraphicsContext.DescriptorPool, nullptr);
        if (m_GraphicsContext.pSurface) vkDestroySurfaceKHR(m_GraphicsContext.Instance, m_GraphicsContext.pSurface, nullptr);
        if (m_GraphicsContext.Device) vkDestroyDevice(m_GraphicsContext.Device, nullptr);
        if (m_GraphicsContext.Instance) vkDestroyInstance(m_GraphicsContext.Instance, nullptr);

        m_GraphicsContext.AcquireFence = VK_NULL_HANDLE;
        m_GraphicsContext.Swapchain = VK_NULL_HANDLE;
        m_GraphicsContext.RenderPass = VK_NULL_HANDLE;
        m_GraphicsContext.DescriptorPool = VK_NULL_HANDLE;
        m_GraphicsContext.Device = VK_NULL_HANDLE;
        m_GraphicsContext.pSurface = VK_NULL_HANDLE;
        m_GraphicsContext.Instance = VK_NULL_HANDLE;
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
        m_GraphicsContext.SwapchainExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

        VkSwapchainCreateInfoKHR swapchainInfo{};
        swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapchainInfo.surface = m_GraphicsContext.pSurface;
        swapchainInfo.minImageCount = 2;
        swapchainInfo.imageFormat = VK_FORMAT_B8G8R8A8_UNORM;
        swapchainInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        swapchainInfo.imageExtent = m_GraphicsContext.SwapchainExtent;
        swapchainInfo.imageArrayLayers = 1;
        swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchainInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapchainInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
        swapchainInfo.clipped = VK_TRUE;
        swapchainInfo.oldSwapchain = oldSwapchain;

        vkCreateSwapchainKHR(m_GraphicsContext.Device, &swapchainInfo, nullptr, &m_GraphicsContext.Swapchain);

        if (oldSwapchain != VK_NULL_HANDLE)
        {
            vkDestroySwapchainKHR(m_GraphicsContext.Device, oldSwapchain, nullptr);
        }

        uint32_t imageCount = 0;
        vkGetSwapchainImagesKHR(m_GraphicsContext.Device, m_GraphicsContext.Swapchain, &imageCount, nullptr);
        m_GraphicsContext.SwapchainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(m_GraphicsContext.Device, m_GraphicsContext.Swapchain, &imageCount, m_GraphicsContext.SwapchainImages.data());

        m_GraphicsContext.SwapchainImageViews.resize(imageCount);
        m_GraphicsContext.Framebuffers.resize(imageCount);

        for (size_t i = 0; i < imageCount; i++)
        {
            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = m_GraphicsContext.SwapchainImages[i];
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = VK_FORMAT_B8G8R8A8_UNORM;
            viewInfo.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

            vkCreateImageView(m_GraphicsContext.Device, &viewInfo, nullptr, &m_GraphicsContext.SwapchainImageViews[i]);

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = m_GraphicsContext.RenderPass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = &m_GraphicsContext.SwapchainImageViews[i];
            framebufferInfo.width = m_GraphicsContext.SwapchainExtent.width;
            framebufferInfo.height = m_GraphicsContext.SwapchainExtent.height;
            framebufferInfo.layers = 1;

            vkCreateFramebuffer(m_GraphicsContext.Device, &framebufferInfo, nullptr, &m_GraphicsContext.Framebuffers[i]);
        }
    }
}
