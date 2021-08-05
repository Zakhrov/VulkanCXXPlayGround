#include "cfx_app.hpp"
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
        CFXRenderSystem cfxRenderSystem{cfxDevice,cfxRenderer.getSwapChainRenderPass()};
        while(!window.shouldClose()){

          auto renderBuffer = cfxRenderer.beginFrame();
          if(renderBuffer.commandBuffer != nullptr){
            cfxRenderer.beginSwapChainRenderPass(renderBuffer.commandBuffer,renderBuffer.deviceMask,renderBuffer.deviceIndex);
            cfxRenderSystem.renderGameObjects(renderBuffer.commandBuffer,cfxGameObjects,renderBuffer.deviceMask);
            cfxRenderer.endSwapChainRenderPass(renderBuffer.commandBuffer,renderBuffer.deviceMask);
            cfxRenderer.endFrame();
          }
           
            glfwPollEvents();
            
        }

        vkDeviceWaitIdle(cfxDevice.device());
    }
    // temporary helper function, creates a 1x1x1 cube centered at offset
std::unique_ptr<CFXModel> createCubeModel(CFXDevice& device, glm::vec3 offset) {
  std::vector<CFXModel::Vertex> vertices{

      // left face (white)
      {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
      {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
      {{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
      {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
      {{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},
      {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},

      // right face (yellow)
      {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
      {{.5f, .5f, .5f}, {.8f, .8f, .1f}},
      {{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
      {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
      {{.5f, .5f, -.5f}, {.8f, .8f, .1f}},
      {{.5f, .5f, .5f}, {.8f, .8f, .1f}},

      // top face (orange, remember y axis points down)
      {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
      {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
      {{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
      {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
      {{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
      {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},

      // bottom face (red)
      {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
      {{.5f, .5f, .5f}, {.8f, .1f, .1f}},
      {{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
      {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
      {{.5f, .5f, -.5f}, {.8f, .1f, .1f}},
      {{.5f, .5f, .5f}, {.8f, .1f, .1f}},

      // nose face (blue)
      {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
      {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
      {{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
      {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
      {{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
      {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},

      // tail face (green)
      {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
      {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
      {{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
      {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
      {{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
      {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},

  };
  for (auto& v : vertices) {
    v.position += offset;
  }
  return std::make_unique<CFXModel>(device, vertices);
}

    void App::loadGameObjects(){
        std::cout << "LOAD MODELS" << std::endl;
        // std::vector<CFXModel::Vertex> vertices {
        //     {{0.0f,-0.5f},{1.0f,0.0f,0.0f}},
        //     {{0.5f,0.5f},{.0f,1.0f,0.0f}},
        //     {{-0.5f,0.5f},{0.0f,0.0f,1.0f}},
        // };
        // sierpinski(vertices, 5, {-0.5f, 0.5f}, {0.5f, 0.5f}, {0.0f, -0.5f});
        // auto cfxModel = std::make_shared<CFXModel>(cfxDevice,vertices);
        

          std::shared_ptr<CFXModel> cfxModel = createCubeModel(cfxDevice, {.0f, .0f, .0f});
          auto cube = CFXGameObject::createGameObject();
          cube.transformComponent.translation = {.0f,.0f,.5f};
          cube.transformComponent.scale = {.5f,.5f,.5f};
          cube.model = cfxModel;

        
        cfxGameObjects.push_back(std::move(cube));
    }

    
  


 

  
  
    
  
  
}