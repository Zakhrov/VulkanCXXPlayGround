#pragma once
#include <string>
#include <vector>
#include "cfx_device.hpp"
namespace cfx {
    struct PipelineConfigInfo{};
    class CFXPipeLine {
        public:
        CFXPipeLine(CFXDevice& device, const PipelineConfigInfo& configInfo, const std::string& vertFilePath, const std::string& fragFilePath);
        ~CFXPipeLine(){}
        CFXPipeLine(const CFXPipeLine&) = delete;
        void operator=(const CFXPipeLine&) = delete;

        static PipelineConfigInfo defaultPipelineConfigInfo(uint32_t width, uint32_t height);

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