#include "cfx_render_system.hpp"
#include<stdexcept>
#include<array>
#include<iostream>
#include<memory>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>


namespace cfx{
  struct SimplePushConstantData{
    glm::mat4 transform{1.f};
    alignas(16) glm::vec3 color;

  };
    CFXRenderSystem::CFXRenderSystem(CFXDevice& device,VkRenderPass renderpass): cfxDevice{device} {
        
        createPipelineLayout();
        createPipeline(renderpass);
        
       
    }
    CFXRenderSystem::~CFXRenderSystem(){
        vkDestroyPipelineLayout(cfxDevice.device(),pipelineLayout,nullptr);
    }
    

    void CFXRenderSystem::renderGameObjects(VkCommandBuffer commandBuffer,std::vector<CFXGameObject> &cfxGameObjects,uint32_t deviceMask){
      cfxPipeLine->bind(commandBuffer);

  for (auto& obj : cfxGameObjects) {
    obj.transformComponent.rotation.y = glm::mod(obj.transformComponent.rotation.y + 0.01f, glm::two_pi<float>());
    obj.transformComponent.rotation.x = glm::mod(obj.transformComponent.rotation.x + 0.005f, glm::two_pi<float>());

    SimplePushConstantData push{};
    push.color = obj.color;
    push.transform = obj.transformComponent.mat4();

    vkCmdPushConstants(
        commandBuffer,
        pipelineLayout,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        0,
        sizeof(SimplePushConstantData),
        &push);
    obj.model->bind(commandBuffer);
    obj.model->draw(commandBuffer);
  }
}
    void CFXRenderSystem::createPipelineLayout(){
      VkPushConstantRange pushConstantRange{};
      pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
      pushConstantRange.offset = 0;
      pushConstantRange.size = sizeof(SimplePushConstantData);
        std::cout << "CREATE PIPELINE LAYOUT"<< std::endl;
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        pipelineLayoutInfo.pushConstantRangeCount = 1;

        if(vkCreatePipelineLayout(cfxDevice.device(),&pipelineLayoutInfo,nullptr,&pipelineLayout) != VK_SUCCESS){
            throw std::runtime_error("failed to create pipeline layout");
        }
    }
    void CFXRenderSystem::createPipeline(VkRenderPass renderpass){
      
        std::cout << "CREATE PIPELINE"<< std::endl;
        PipelineConfigInfo pipelineConfig{};
        CFXPipeLine::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = renderpass;
        pipelineConfig.pipelineLayout = pipelineLayout;
        cfxPipeLine = std::make_unique<CFXPipeLine>(cfxDevice,pipelineConfig,
        "shaders/simple_shader.vert.spv",
        "shaders/simple_shader.frag.spv");

    }
  


 

  
  
    
  
  
}