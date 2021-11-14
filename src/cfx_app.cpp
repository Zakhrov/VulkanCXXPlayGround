#include "cfx_app.hpp"
#include "cfx_render_system.hpp"
#include "cfx_camera.hpp"
#include "cfx_buffer.hpp"
#include "keyboard_movement_controller.hpp"
#include<stdexcept>
#include<array>
#include<iostream>
#include<memory>
#include <chrono>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>


namespace cfx{
  struct GlobalUbo {
    glm::mat4 projectionView{1.f};
    glm::vec3 lightDirection = glm::normalize(glm::vec3{1.f, -3.f, -1.f});

  };
 
    App::App(){
        cfxDescriptorPools.resize(cfxDevice.getDevicesinDeviceGroup());
        for(int deviceIndex=0; deviceIndex < cfxDevice.getDevicesinDeviceGroup(); deviceIndex++ ){
          cfxDescriptorPools[deviceIndex] = CFXDescriptorPool::Builder(cfxDevice)
          .setMaxSets(CFXSwapChain::MAX_FRAMES_IN_FLIGHT)
          .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,CFXSwapChain::MAX_FRAMES_IN_FLIGHT)
          .build(deviceIndex);
        }
        loadGameObjects();
       
       
    }
    App::~App(){
       
    }
    void App::run(){


      std::vector<std::vector<std::unique_ptr<CFXBuffer>>> uboBuffers(cfxDevice.getDevicesinDeviceGroup());
      std::vector<std::vector<VkDescriptorSet>> cfxGlobalDescriptorSets(cfxDevice.getDevicesinDeviceGroup());
      std::vector<std::unique_ptr<CFXDescriptorSetLayout>> cfxSetLayouts(cfxDevice.getDevicesinDeviceGroup());
      
      for(int deviceIndex=0; deviceIndex < uboBuffers.size(); deviceIndex++){
        std::cout<< "RESIZING UBO FOR DEVICE " << deviceIndex << std::endl;
        uboBuffers[deviceIndex].resize(CFXSwapChain::MAX_FRAMES_IN_FLIGHT);
        cfxGlobalDescriptorSets[deviceIndex].resize(CFXSwapChain::MAX_FRAMES_IN_FLIGHT);
        for(int i=0; i < uboBuffers[deviceIndex].size(); i++){
          std::cout<< "MAPPING UBO FOR FRAME " << i << " ON DEVICE "  << deviceIndex << std::endl;
            uboBuffers[deviceIndex][i] = std::make_unique<CFXBuffer>(cfxDevice,sizeof(GlobalUbo),1,VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,(int)deviceIndex);
            uboBuffers[deviceIndex][i]->map();
            std::cout<< "MAPPED UBO FOR FRAME " << i << " ON DEVICE "  << deviceIndex << std::endl;
          }
          cfxSetLayouts[deviceIndex] = CFXDescriptorSetLayout::Builder(cfxDevice).addBinding(0,VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_SHADER_STAGE_VERTEX_BIT).build(deviceIndex);
          
          
          for(int i=0; i< cfxGlobalDescriptorSets[deviceIndex].size(); i++){

            std::cout<< "WRITING DESCRIPTOR SET " << i << " ON DEVICE " << deviceIndex << std::endl;
            auto bufferInfo = uboBuffers[deviceIndex][i]->descriptorInfo();
            CFXDescriptorWriter(*cfxSetLayouts[deviceIndex],*cfxDescriptorPools[deviceIndex]).writeBuffer(0,&bufferInfo).build(cfxGlobalDescriptorSets[deviceIndex][i],deviceIndex);
            std::cout<< "WROTE DESCRIPTOR SET " << i << " ON DEVICE " << deviceIndex << std::endl;
          }
          std::cout<< "FINISHED DESCRIPTOR SET ALLOCATION FOR DEVICE "  << deviceIndex << std::endl;
      }
          

      
      // std::cout << "CREATE RENDER SYSTEM"<< std::endl;
        CFXRenderSystem cfxRenderSystem{cfxDevice,cfxRenderer.getSwapChainRenderPasses(),cfxSetLayouts};
        CFXCamera camera{};
       
        auto viewerObject = CFXGameObject::createGameObject();
        KeyboardMovementController cameraController{};
        
        
        auto currentTime = std::chrono::high_resolution_clock::now();
        
        while(!window.shouldClose()){
          
          glfwPollEvents();
          auto newTime = std::chrono::high_resolution_clock::now();
          float frameTime = std::chrono::duration<float,std::chrono::milliseconds::period>(newTime-currentTime).count();
          currentTime = newTime;
          cameraController.moveInPlaneXZ(window.getGLFWwindow(),frameTime,viewerObject);
          camera.setViewYXZ(viewerObject.transformComponent.translation,viewerObject.transformComponent.rotation);
          float aspect = cfxRenderer.getAspectRatio();
          // camera.setOrthographicProjection(-aspect,aspect,-1,1,-1,1);
          camera.setPerspectiveProjection(glm::radians(50.f),aspect,0.1f,10.f);
          

          auto frameTimeBegin = std::chrono::high_resolution_clock::now();
          auto renderBuffer = cfxRenderer.beginFrame();
          
          if(renderBuffer.commandBuffer != nullptr){
            int frameIndex = cfxRenderer.getFrameIndex();
            FrameInfo frameInfo{frameIndex,frameTime,renderBuffer.commandBuffer,camera,renderBuffer.deviceIndex,cfxGlobalDescriptorSets[renderBuffer.deviceIndex][frameIndex]};
            GlobalUbo globalUbo{};
            globalUbo.projectionView = camera.getProjection() * camera.getView();
            uboBuffers[renderBuffer.deviceIndex][frameIndex]->writeToBuffer(&globalUbo);
            uboBuffers[renderBuffer.deviceIndex][frameIndex]->flush();

            
            cfxRenderer.beginSwapChainRenderPass(renderBuffer.commandBuffer,renderBuffer.deviceMask,renderBuffer.deviceIndex);
            cfxRenderSystem.renderGameObjects(frameInfo,cfxGameObjects);
            cfxRenderer.endSwapChainRenderPass(renderBuffer.commandBuffer,renderBuffer.deviceMask,renderBuffer.deviceIndex);
            cfxRenderer.endFrame(renderBuffer.deviceIndex);
            vkDeviceWaitIdle(cfxDevice.device(renderBuffer.deviceIndex));
          }
          auto frameTimeEnd = std::chrono::high_resolution_clock::now();
          float renderFrameTime = std::chrono::duration<float,std::chrono::nanoseconds::period>(frameTimeEnd - frameTimeBegin).count();
          // std::cout << "FRAME TIME = "<< renderFrameTime << " ms" << std::endl;
          std::string framerateString = std::to_string(1000000000 / renderFrameTime) + " fps " + std::to_string(renderFrameTime) +  " ns";
          
          window.setWindowName(framerateString );
          
          
           
            
            
        }

      

        
    }
   



    void App::loadGameObjects(){
        
        

          std::shared_ptr<CFXModel> cfxModel = CFXModel::createModelFromFile(cfxDevice, "models/smooth_vase.obj");
          auto smoothVase = CFXGameObject::createGameObject();
          smoothVase.transformComponent.translation = {-.5f,.0f,2.5f};
          smoothVase.transformComponent.scale = glm::vec3{3.f,1.5f,3.f};
          smoothVase.model = cfxModel;
          cfxGameObjects.push_back(std::move(smoothVase));
          cfxModel = CFXModel::createModelFromFile(cfxDevice, "models/flat_vase.obj");
          auto flatVase = CFXGameObject::createGameObject();
          flatVase.transformComponent.translation = {.5f,.0f,2.5f};
          flatVase.transformComponent.scale = glm::vec3{3.f,1.5f,3.f};
          flatVase.model = cfxModel;
          cfxGameObjects.push_back(std::move(flatVase));

        
        
    }

    
  


 

  
  
    
  
  
}