#include "cfx_pipeline.hpp"
#include<fstream>
#include <iostream>
namespace cfx{
    CFXPipeLine::CFXPipeLine(CFXDevice& device, const PipelineConfigInfo& configInfo, const std::string& vertFilePath, const std::string& fragFilePath):cfxDevice{device}{
        createGraphicsPipeLine(configInfo,vertFilePath,fragFilePath);
    }
    std::vector<char> CFXPipeLine::readFile(const std::string& filepath){
        std::ifstream file{filepath, std::ios::ate | std::ios::binary};
        if(!file.is_open()){
            throw std::runtime_error("failed to open file: " + filepath);
        }
        size_t fileSize = static_cast<size_t>(file.tellg());
        std::vector<char> buffer(fileSize);
        file.seekg(0);
        file.read(buffer.data(),fileSize);
        file.close();
        return buffer;
        

    }
    void CFXPipeLine::createGraphicsPipeLine(const PipelineConfigInfo& configInfo,const std::string& vertFilePath, const std::string& fragFilePath){
        auto vertCode = readFile(vertFilePath);
        auto fragCode = readFile(fragFilePath);

        std::cout << "Vertex Shader code Size: " << vertCode.size() << std::endl;
        std::cout << "Fragment Shader code Size: " << fragCode.size() << std::endl;
    }

    void CFXPipeLine::createShaderModule(const std::vector<char>& code, VkShaderModule * shaderModule){
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
        if(vkCreateShaderModule(cfxDevice.device(),&createInfo,nullptr,shaderModule) != VK_SUCCESS){
            throw std::runtime_error("Failed to create shader module");
        }

    }
    PipelineConfigInfo CFXPipeLine::defaultPipelineConfigInfo(uint32_t width, uint32_t height){
        PipelineConfigInfo configInfo{};

        return configInfo;
    }
}