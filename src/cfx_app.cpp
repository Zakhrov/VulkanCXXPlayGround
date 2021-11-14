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
    glm::vec3 lightDirection = glm::normalize(glm::vec3{1.f,-3.f,1.f});

  };
 
    App::App(){
        loadGameObjects();
       
       
    }
    App::~App(){
       
    }
    void App::run(){

      
      // std::cout << "CREATE RENDER SYSTEM"<< std::endl;
        CFXRenderSystem cfxRenderSystem{cfxDevice,cfxRenderer.getSwapChainRenderPasses()};
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
          std::vector<std::unique_ptr<CFXBuffer>> uboBuffers(CFXSwapChain::MAX_FRAMES_IN_FLIGHT);
          for(int i=0; i < uboBuffers.size(); i++){
            uboBuffers[i] = std::make_unique<CFXBuffer>(cfxDevice,sizeof(GlobalUbo),1,VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,(int)renderBuffer.deviceIndex,cfxDevice.properties[(int)renderBuffer.deviceIndex].limits.minUniformBufferOffsetAlignment);
            uboBuffers[i]->map();
          }
          if(renderBuffer.commandBuffer != nullptr){
            int frameIndex = cfxRenderer.getFrameIndex();
            FrameInfo frameInfo{frameIndex,frameTime,renderBuffer.commandBuffer,camera,renderBuffer.deviceIndex};
            GlobalUbo globalUbo{};
            globalUbo.projectionView = camera.getProjection() * camera.getView();
            uboBuffers[frameIndex]->writeToBuffer(&globalUbo);
            uboBuffers[frameIndex]->flush();
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