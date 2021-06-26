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

          if(auto commandBuffer = cfxRenderer.beginFrame()){
            cfxRenderer.beginSwapChainRenderPass(commandBuffer);
            cfxRenderSystem.renderGameObjects(commandBuffer,cfxGameObjects);
            cfxRenderer.endSwapChainRenderPass(commandBuffer);
            cfxRenderer.endFrame();
          }
           
            glfwPollEvents();
            
        }

        vkDeviceWaitIdle(cfxDevice.device());
    }

    void App::loadGameObjects(){
        std::cout << "LOAD MODELS" << std::endl;
        std::vector<CFXModel::Vertex> vertices {
            {{0.0f,-0.5f},{1.0f,0.0f,0.0f}},
            {{0.5f,0.5f},{.0f,1.0f,0.0f}},
            {{-0.5f,0.5f},{0.0f,0.0f,1.0f}},
        };
        // sierpinski(vertices, 5, {-0.5f, 0.5f}, {0.5f, 0.5f}, {0.0f, -0.5f});
        auto cfxModel = std::make_shared<CFXModel>(cfxDevice,vertices);
        auto triangle = CFXGameObject::createGameObject();
        triangle.model = cfxModel;
        triangle.color = {.1f,.8f,.1f};
        triangle.transform2d.translation.x = .2f;
        triangle.transform2d.scale = {2.f,.5f};
        triangle.transform2d.rotation = .25f * glm::two_pi<float>();
        cfxGameObjects.push_back(std::move(triangle));
    }

    
  


 

  
  
    
  
  
}