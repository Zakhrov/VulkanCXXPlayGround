#pragma once
#include <string>
#include <vector>
#include "cfx_device.hpp"
namespace cfx {
    struct PipelineConfigInfo{
       PipelineConfigInfo(const PipelineConfigInfo&) = delete;
  PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

  VkPipelineViewportStateCreateInfo viewportInfo;
  VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
  VkPipelineRasterizationStateCreateInfo rasterizationInfo;
  VkPipelineMultisampleStateCreateInfo multisampleInfo;
  VkPipelineColorBlendAttachmentState colorBlendAttachment;
  VkPipelineColorBlendStateCreateInfo colorBlendInfo;
  VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
  std::vector<VkDynamicState> dynamicStateEnables;
  VkPipelineDynamicStateCreateInfo dynamicStateInfo;
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

        static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo);

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