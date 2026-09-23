#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <string>

namespace Daedalus {
    class TrianglePipeline
    {
        public:
            TrianglePipeline() = default;
            ~TrianglePipeline() = default;

            bool Initialize(VkDevice device, VkRenderPass renderPass, VkExtent2D extent);
            void RecordDraw(VkCommandBuffer commandBuffer, VkExtent2D extent);
            void Cleanup(VkDevice device);

        private:
            static std::vector<char> ReadFile(const std::string& filename);
            static VkShaderModule CreateShaderModule(VkDevice device, const std::vector<char>& code);

            VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
            VkPipeline m_Pipeline = VK_NULL_HANDLE;
    };
}