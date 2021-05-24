#pragma once
#include <string>
#include <vector>
#include "cfx_device.hpp"
namespace cfx {
    struct PipelineConfigInfo{
        VkViewport viewport;
        VkRect2D scissor;
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
        VkPipelineRasterizationStateCreateInfo rasterizationInfo;
        VkPipelineMultisampleStateCreateInfo multisampleInfo;
        VkPipelineColorBlendAttachmentState colorBlendAttachment;
        VkPipelineColorBlendStateCreateInfo colorBlendInfo;
        VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
        VkPipelineLayout pipelineLayout = nullptr;
        VkRenderPass renderPass = nullptr;
        uint32_t subpass = 0;
    };
    class CFXPipeLine {
        public:
        CFXPipeLine(CFXDevice& device, const PipelineConfigInfo& configInfo, const std::string& vertFilePath, const std::string& fragFilePath);
        ~CFXPipeLine();
        CFXPipeLine(const CFXPipeLine&) = delete;
        void operator=(const CFXPipeLine&) = delete;

        static PipelineConfigInfo defaultPipelineConfigInfo(uint32_t width, uint32_t height);

        void bind(VkCommandBuffer commandBuffer);

        private:
        static std::vector<char> readFile(const std::string& filepath);

        void createGraphicsPipeLine(const PipelineConfigInfo& configInfo,const std::string& vertFilePath, const std::string& fragFilePath);
        void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);
        CFXDevice& cfxDevice;
        VkPipeline graphicsPipeline;
        VkShaderModule vertShaderModule;
        VkShaderModule fragShaderModule;
    };
}