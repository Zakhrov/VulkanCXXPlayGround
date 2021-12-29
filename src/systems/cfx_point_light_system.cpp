#include "cfx_point_light_system.hpp"
#include<stdexcept>
#include<array>
#include<iostream>
#include<memory>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>


namespace cfx{
  
    CFXPointLightSystem::CFXPointLightSystem(CFXDevice& device,std::vector<VkRenderPass> renderPasses,std::vector<std::unique_ptr<CFXDescriptorSetLayout>> &cfxSetLayouts): cfxDevice{device} {
        pipelineLayout.resize(cfxDevice.getDevicesinDeviceGroup());
        cfxPipeLines.resize(cfxDevice.getDevicesinDeviceGroup());
        for(int deviceIndex = 0; deviceIndex < cfxDevice.getDevicesinDeviceGroup(); deviceIndex++ ){
        createPipelineLayout(cfxSetLayouts[deviceIndex]->getDescriptorSetLayout(),deviceIndex);
        createPipeline(renderPasses[deviceIndex],deviceIndex);
        }
        
        
       
    }
    CFXPointLightSystem::~CFXPointLightSystem(){
        for(int i = 0; i < cfxDevice.getDevicesinDeviceGroup(); i++){
          vkDestroyPipelineLayout(cfxDevice.device(i),pipelineLayout[i],nullptr);
          
        }
        
    }
    

    void CFXPointLightSystem::render(FrameInfo &frameInfo){
      // std::cout << "RENDER GAME OBJECTS ON " << cfxDevice.getDeviceName(deviceIndex) << std::endl;
      cfxPipeLines[frameInfo.deviceIndex]->bind(frameInfo.commandBuffer);
      vkCmdBindDescriptorSets(frameInfo.commandBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS,pipelineLayout[frameInfo.deviceIndex],0,1,&frameInfo.globalDescriptorSet,0,nullptr);
      vkCmdDraw(frameInfo.commandBuffer,6,1,0,0);
}
    void CFXPointLightSystem::createPipelineLayout(VkDescriptorSetLayout descriptorSetLayout, int deviceIndex){
    //   VkPushConstantRange pushConstantRange{};
    //   pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    //   pushConstantRange.offset = 0;
    //   pushConstantRange.size = sizeof(SimplePushConstantData);
      std::vector<VkDescriptorSetLayout> layouts{descriptorSetLayout};
        // std::cout << "CREATE PIPELINE LAYOUT  "  << std::endl;
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = layouts.size();
        pipelineLayoutInfo.pSetLayouts = layouts.data();
        pipelineLayoutInfo.pPushConstantRanges = nullptr;
        pipelineLayoutInfo.pushConstantRangeCount = 0;

        if(vkCreatePipelineLayout(cfxDevice.device(deviceIndex),&pipelineLayoutInfo,nullptr,&pipelineLayout[deviceIndex]) != VK_SUCCESS){
            throw std::runtime_error("failed to create pipeline layout");
        }
      
    }
    void CFXPointLightSystem::createPipeline(VkRenderPass renderpass,int deviceIndex){
      
        // std::cout << "CREATE PIPELINE " << std::endl;
        PipelineConfigInfo pipelineConfig{};
        CFXPipeLine::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.attributeDescriptions.clear();
        pipelineConfig.bindingDescriptions.clear();
        
        pipelineConfig.renderPass = renderpass;
        pipelineConfig.pipelineLayout = pipelineLayout[deviceIndex];
        cfxPipeLines[deviceIndex] = std::make_unique<CFXPipeLine>(cfxDevice,pipelineConfig,
        "shaders/point_light.vert.spv",
        "shaders/point_light.frag.spv",deviceIndex);
        // std::cout << "CREATE PIPELINE END " << std::endl;

    }
  


 

  
  
    
  
  
}