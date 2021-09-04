#include "cfx_app.hpp"
#include "cfx_render_system.hpp"
#include "cfx_camera.hpp"
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
  struct SimplePushConstantData{
    glm::mat2 transform{1.f};
    glm::vec2 offset;
    alignas(16) glm::vec3 color;

  };
    App::App(){
        loadGameObjects();
       
       
    }
    App::~App(){
       
    }
    void App::run(){
      std::cout << "CREATE RENDER SYSTEM"<< std::endl;
        CFXRenderSystem cfxRenderSystem{cfxDevice,cfxRenderer.getSwapChainRenderPasses()};
        CFXCamera camera{};
        // camera.setViewDirection(glm::vec3(0.f),glm::vec3(0.5f,0.f,1.f));
        // camera.setViewTarget(glm::vec3(-1.f,-2.f,2.f),glm::vec3(0.f,0.f,2.5f));

        auto viewerObject = CFXGameObject::createGameObject();
        KeyboardMovementController cameraController{};
        
        std::cout << "CREATED RENDER SYSTEM"<< std::endl;
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
          


          auto renderBuffer = cfxRenderer.beginFrame();
          if(renderBuffer.commandBuffer != nullptr){
            cfxRenderer.beginSwapChainRenderPass(renderBuffer.commandBuffer,renderBuffer.deviceMask,renderBuffer.deviceIndex);
            cfxRenderSystem.renderGameObjects(renderBuffer.commandBuffer,cfxGameObjects,renderBuffer.deviceMask,camera);
            cfxRenderer.endSwapChainRenderPass(renderBuffer.commandBuffer,renderBuffer.deviceMask,renderBuffer.deviceIndex);
            cfxRenderer.endFrame();
            vkDeviceWaitIdle(cfxDevice.device(renderBuffer.deviceIndex));
          }
           
            
            
        }

        // for(int i=0; i < cfxDevice.getDevicesinDeviceGroup(); i++){
          

        // }

        
    }
    // temporary helper function, creates a 1x1x1 cube centered at offset
// std::unique_ptr<CFXModel> createCubeModel(CFXDevice& device, glm::vec3 offset) {
//   CFXModel::Builder modelBuilder{};

//    modelBuilder.vertices = {
//       // left face (white)
//       {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
//       {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
//       {{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
//       {{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},
 
//       // right face (yellow)
//       {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
//       {{.5f, .5f, .5f}, {.8f, .8f, .1f}},
//       {{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
//       {{.5f, .5f, -.5f}, {.8f, .8f, .1f}},
 
//       // top face (orange, remember y axis points down)
//       {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
//       {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
//       {{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
//       {{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
 
//       // bottom face (red)
//       {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
//       {{.5f, .5f, .5f}, {.8f, .1f, .1f}},
//       {{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
//       {{.5f, .5f, -.5f}, {.8f, .1f, .1f}},
 
//       // nose face (blue)
//       {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
//       {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
//       {{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
//       {{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
 
//       // tail face (green)
//       {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
//       {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
//       {{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
//       {{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
//   };
//   for (auto& v : modelBuilder.vertices) {
//     v.position += offset;
//   }
 
//   modelBuilder.indices = {0,  1,  2,  0,  3,  1,  4,  5,  6,  4,  7,  5,  8,  9,  10, 8,  11, 9,
//                           12, 13, 14, 12, 15, 13, 16, 17, 18, 16, 19, 17, 20, 21, 22, 20, 23, 21};
  
//   return std::make_unique<CFXModel>(device, modelBuilder);
//   // return nullptr;
// }



    void App::loadGameObjects(){
        std::cout << "LOAD MODELS" << std::endl;
        // std::vector<CFXModel::Vertex> vertices {
        //     {{0.0f,-0.5f},{1.0f,0.0f,0.0f}},
        //     {{0.5f,0.5f},{.0f,1.0f,0.0f}},
        //     {{-0.5f,0.5f},{0.0f,0.0f,1.0f}},
        // };
        // sierpinski(vertices, 5, {-0.5f, 0.5f}, {0.5f, 0.5f}, {0.0f, -0.5f});
        // auto cfxModel = std::make_shared<CFXModel>(cfxDevice,vertices);
        

          std::shared_ptr<CFXModel> cfxModel = CFXModel::createModelFromFile(cfxDevice, "models/smooth_vase.obj");
          auto gameObject = CFXGameObject::createGameObject();
          gameObject.transformComponent.translation = {.0f,.0f,2.5f};
          gameObject.transformComponent.scale = glm::vec3{3.f};
          gameObject.model = cfxModel;

        
        cfxGameObjects.push_back(std::move(gameObject));
    }

    
  


 

  
  
    
  
  
}