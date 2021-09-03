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
    glm::mat4 modelMatrix{1.f};
    // alignas(16) glm::vec3 color;


  };
    CFXRenderSystem::CFXRenderSystem(CFXDevice& device,std::vector<VkRenderPass> renderpasses): cfxDevice{device} {
        pipelineLayouts.resize(cfxDevice.getDevicesinDeviceGroup());
        cfxPipeLines.resize(cfxDevice.getDevicesinDeviceGroup());
        for(int i = 0; i < cfxDevice.getDevicesinDeviceGroup(); i++){
          createPipelineLayout(i);
          if(renderpasses[i] != nullptr){
            createPipeline(renderpasses[i],i);
            
          }else{
            std::cout << "RENDERPASS " << i << " IS NULL" << std::endl;
          }
          
        }
        
        
       
    }
    CFXRenderSystem::~CFXRenderSystem(){
        for(int i = 0; i < cfxDevice.getDevicesinDeviceGroup(); i++){
          vkDestroyPipelineLayout(cfxDevice.device(i),pipelineLayouts[i],nullptr);
        }
    }
    

    void CFXRenderSystem::renderGameObjects(VkCommandBuffer commandBuffer,std::vector<CFXGameObject> &cfxGameObjects,uint32_t deviceIndex, const CFXCamera camera){
      // std::cout << "RENDER GAME OBJECTS"<< std::endl;
      cfxPipeLines[deviceIndex]->bind(commandBuffer);
      auto projectionView = camera.getProjection() * camera.getView();

  for (auto& obj : cfxGameObjects) {
    // obj.transformComponent.rotation.y = glm::mod(obj.transformComponent.rotation.y + 0.01f, glm::two_pi<float>());
    // obj.transformComponent.rotation.x = glm::mod(obj.transformComponent.rotation.x + 0.005f, glm::two_pi<float>());

    SimplePushConstantData push{};
    auto  modelMatrix = obj.transformComponent.mat4();

    push.transform = projectionView * modelMatrix;
    push.modelMatrix = modelMatrix;

    vkCmdPushConstants(
        commandBuffer,
        pipelineLayouts[deviceIndex],
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        0,
        sizeof(SimplePushConstantData),
        &push);
    obj.model->bind(commandBuffer,deviceIndex);
    obj.model->draw(commandBuffer,deviceIndex);
    // std::cout << "RENDER GAME OBJECTS END"<< std::endl;
  }
}
    void CFXRenderSystem::createPipelineLayout(int deviceIndex){
      VkPushConstantRange pushConstantRange{};
      pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
      pushConstantRange.offset = 0;
      pushConstantRange.size = sizeof(SimplePushConstantData);
        std::cout << "CREATE PIPELINE LAYOUT  " << deviceIndex << std::endl;
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        pipelineLayoutInfo.pushConstantRangeCount = 1;

        if(vkCreatePipelineLayout(cfxDevice.device(deviceIndex),&pipelineLayoutInfo,nullptr,&pipelineLayouts[deviceIndex]) != VK_SUCCESS){
            throw std::runtime_error("failed to create pipeline layout");
        }
      
    }
    void CFXRenderSystem::createPipeline(VkRenderPass renderpass,int deviceIndex){
      
        std::cout << "CREATE PIPELINE "<< deviceIndex << std::endl;
        PipelineConfigInfo pipelineConfig{};
        CFXPipeLine::defaultPipelineConfigInfo(pipelineConfig,deviceIndex);
        pipelineConfig.renderPass = renderpass;
        pipelineConfig.pipelineLayout = pipelineLayouts[deviceIndex];
        cfxPipeLines[deviceIndex] = new CFXPipeLine(cfxDevice,pipelineConfig,
        "shaders/simple_shader.vert.spv",
        "shaders/simple_shader.frag.spv",deviceIndex);
        std::cout << "CREATE PIPELINE END "<< deviceIndex << std::endl;

    }
  


 

  
  
    
  
  
}