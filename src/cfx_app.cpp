#include "cfx_app.hpp"
#include "systems/cfx_render_system.hpp"
#include "systems/cfx_point_light_system.hpp"
#include "cfx_camera.hpp"
#include "cfx_buffer.hpp"
#include "keyboard_movement_controller.hpp"
#include <stdexcept>
#include <array>
#include <iostream>
#include <memory>
#include <chrono>
#include <sstream>
#include <iterator>
#include <random>


#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/string_cast.hpp>

namespace cfx
{
 

  App::App()
  {
    textures.resize(cfxDevice.getDevicesinDeviceGroup());
    cfxDescriptorPools.resize(cfxDevice.getDevicesinDeviceGroup());
    for (int deviceIndex = 0; deviceIndex < cfxDevice.getDevicesinDeviceGroup(); deviceIndex++)
    {
      textures[deviceIndex] = new Texture(deviceIndex,cfxDevice,"textures/texture_1.tga");
      cfxDescriptorPools[deviceIndex] = CFXDescriptorPool::Builder(cfxDevice)
                                            .setMaxSets(CFXSwapChain::MAX_FRAMES_IN_FLIGHT)
                                            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, CFXSwapChain::MAX_FRAMES_IN_FLIGHT)
                                            .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,CFXSwapChain::MAX_FRAMES_IN_FLIGHT)
                                            .build(deviceIndex);
    }
    loadGameObjects();
  }
  App::~App()
  {
    for(int i=0; i < textures.size(); i++){
      delete textures[i];
    }
   
  }
  void App::run()
  {

    std::vector<std::vector<std::unique_ptr<CFXBuffer>>> uboBuffers(cfxDevice.getDevicesinDeviceGroup());
    std::vector<std::vector<VkDescriptorSet>> cfxGlobalDescriptorSets(cfxDevice.getDevicesinDeviceGroup());
    std::vector<std::unique_ptr<CFXDescriptorSetLayout>> cfxSetLayouts(cfxDevice.getDevicesinDeviceGroup());
    
    std::string framerateString = "Vulkan Window";

    for (int deviceIndex = 0; deviceIndex < uboBuffers.size(); deviceIndex++)
    {
      // std::cout<< "RESIZING UBO FOR DEVICE " << deviceIndex << std::endl;
      uboBuffers[deviceIndex].resize(CFXSwapChain::MAX_FRAMES_IN_FLIGHT);
      cfxGlobalDescriptorSets[deviceIndex].resize(CFXSwapChain::MAX_FRAMES_IN_FLIGHT);
      for (int i = 0; i < uboBuffers[deviceIndex].size(); i++)
      {
        // std::cout<< "MAPPING UBO FOR FRAME " << i << " ON DEVICE "  << deviceIndex << std::endl;
        uboBuffers[deviceIndex][i] = std::make_unique<CFXBuffer>(cfxDevice, sizeof(GlobalUbo), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, (int)deviceIndex);
        uboBuffers[deviceIndex][i]->map();
        // std::cout<< "MAPPED UBO FOR FRAME " << i << " ON DEVICE "  << deviceIndex << std::endl;
      }
      cfxSetLayouts[deviceIndex] = CFXDescriptorSetLayout::Builder(cfxDevice)
      .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
      .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_FRAGMENT_BIT)
      .build(deviceIndex);
      
      

      

      for (int i = 0; i < cfxGlobalDescriptorSets[deviceIndex].size(); i++)
      {

      
      VkDescriptorImageInfo imageInfo{};
      imageInfo.imageLayout = textures[deviceIndex]->getImageLayout();
      imageInfo.imageView = textures[deviceIndex]->getImageView();
      imageInfo.sampler = textures[deviceIndex]->getSampler();

        
        auto bufferInfo = uboBuffers[deviceIndex][i]->descriptorInfo();
        CFXDescriptorWriter(*cfxSetLayouts[deviceIndex], *cfxDescriptorPools[deviceIndex]).writeBuffer(0, &bufferInfo)
        .writeImage(1,&imageInfo)
        .build(cfxGlobalDescriptorSets[deviceIndex][i], deviceIndex);
        
      }
      
    }

    
    CFXRenderSystem cfxRenderSystem{cfxDevice, cfxRenderer.getSwapChainRenderPasses(), cfxSetLayouts};
    CFXPointLightSystem cfxPointLightSystem{cfxDevice, cfxRenderer.getSwapChainRenderPasses(), cfxSetLayouts};
    CFXCamera camera{};

    auto viewerObject = CFXGameObject::createGameObject();
    viewerObject.transformComponent.translation.z = -7.5f;
    viewerObject.transformComponent.translation.y = -1.5f;
    KeyboardMovementController cameraController{};

    auto currentTime = std::chrono::high_resolution_clock::now();
    auto pollTimeStart = std::chrono::high_resolution_clock::now();
    std::string deviceName = cfxDevice.getDeviceName(0);
    std::vector<std::string> framerateStrings(cfxDevice.getDevicesinDeviceGroup());
    std::vector<float> frameTimes(cfxDevice.getDevicesinDeviceGroup());
    float totalFrameTime = 0;
    int frameCounter = 0;

    while (!window.shouldClose())
    {

      glfwPollEvents();
      auto newTime = std::chrono::high_resolution_clock::now();

      float frameTime = std::chrono::duration<float, std::chrono::milliseconds::period>(newTime - currentTime).count();
      currentTime = newTime;
      cameraController.moveInPlaneXZ(window.getGLFWwindow(), frameTime, viewerObject);
      camera.setViewYXZ(viewerObject.transformComponent.translation, viewerObject.transformComponent.rotation);
      float aspect = cfxRenderer.getAspectRatio();
      // camera.setOrthographicProjection(-aspect,aspect,-1,1,-1,1);
      camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f);

      auto renderBuffer = cfxRenderer.beginFrame();

      if (renderBuffer.commandBuffer != nullptr)
      {
        auto frameTimeStart = std::chrono::high_resolution_clock::now();
        int frameIndex = cfxRenderer.getFrameIndex();
        deviceName = cfxDevice.getDeviceName(renderBuffer.deviceIndex);

        FrameInfo frameInfo{frameIndex, frameTime, renderBuffer.commandBuffer, camera, renderBuffer.deviceIndex, cfxGlobalDescriptorSets[renderBuffer.deviceIndex][frameIndex], cfxGameObjects};
        GlobalUbo globalUbo{};
        globalUbo.projection = camera.getProjection();
        globalUbo.view = camera.getView();
        globalUbo.inverseView = camera.getInverseView();

        cfxPointLightSystem.update(frameInfo,globalUbo);

        uboBuffers[renderBuffer.deviceIndex][frameIndex]->writeToBuffer(&globalUbo);

        uboBuffers[renderBuffer.deviceIndex][frameIndex]->flush();

        cfxRenderer.beginSwapChainRenderPass(renderBuffer.commandBuffer, renderBuffer.deviceMask, renderBuffer.deviceIndex);

        
        // order here matters. We want to render the game objects before the point lights
        cfxRenderSystem.renderGameObjects(frameInfo);
        cfxPointLightSystem.render(frameInfo);

        cfxRenderer.endSwapChainRenderPass(renderBuffer.commandBuffer, renderBuffer.deviceMask, renderBuffer.deviceIndex);

        cfxRenderer.endFrame(renderBuffer.deviceIndex);

        vkDeviceWaitIdle(cfxDevice.device(renderBuffer.deviceIndex));
        auto frameTimeEnd = std::chrono::high_resolution_clock::now();
        float renderFrameTime = std::chrono::duration<float, std::chrono::milliseconds::period>(frameTimeEnd - frameTimeStart).count();
        
        framerateStrings[renderBuffer.deviceIndex] = "GPU " + cfxDevice.getDeviceName(renderBuffer.deviceIndex) + "  Frame time "+ std::to_string(renderFrameTime) + " ms ";
        totalFrameTime += renderFrameTime;
        frameCounter++;

        
        
        
      }

      auto pollTimeEnd = std::chrono::high_resolution_clock::now();

      float pollInterval = std::chrono::duration<float, std::chrono::milliseconds::period>(pollTimeEnd - pollTimeStart).count();
      

      if (pollInterval >= 100)
      {
        std::ostringstream oss;
        std::copy(framerateStrings.begin(), framerateStrings.end(), std::ostream_iterator<std::string>(oss, " ; "));

        std::string result(oss.str());
        // framerateString = result;
        
        
        float avgFrameTime = totalFrameTime / frameCounter;
        framerateString = "Average " + std::to_string(1000 / avgFrameTime ) + " FPS " + std::to_string(avgFrameTime ) + " ms Polled Frames: " + std::to_string(frameCounter) + " Polled time " + std::to_string(totalFrameTime) +" ms ";
        framerateString += result;
        totalFrameTime = 0; 
        frameCounter = 0;


        pollTimeStart = pollTimeEnd;
      }

      window.setWindowName(framerateString);
    }
  }

  void App::loadGameObjects()
  {

    std::shared_ptr<CFXModel> cfxModel = CFXModel::createModelFromFile(cfxDevice, "models/smooth_vase.obj");
    auto smoothVase = CFXGameObject::createGameObject();
    smoothVase.transformComponent.translation = {-.5f, .5f, 0.f};
    smoothVase.transformComponent.scale = glm::vec3{3.f, 1.5f, 3.f};
    smoothVase.model = cfxModel;
    cfxGameObjects.emplace(smoothVase.getId(), std::move(smoothVase));
    cfxModel = CFXModel::createModelFromFile(cfxDevice, "models/flat_vase.obj");
    auto flatVase = CFXGameObject::createGameObject();
    flatVase.transformComponent.translation = {.5f, .5f, 0.f};
    flatVase.transformComponent.scale = glm::vec3{3.f, 1.5f, 3.f};
    flatVase.model = cfxModel;
    cfxGameObjects.emplace(flatVase.getId(), std::move(flatVase));

    cfxModel = CFXModel::createModelFromFile(cfxDevice, "models/quad.obj");
    auto floor = CFXGameObject::createGameObject();
    floor.transformComponent.translation = {0.f, .5f, 0.f};
    floor.transformComponent.scale = glm::vec3{10.f, 1.f, 10.f};
    floor.model = cfxModel;
    cfxGameObjects.emplace(floor.getId(), std::move(floor));

  //    std::vector<glm::vec3> lightColors{
  //     {1.f, .1f, .1f},
  //     {.1f, .1f, 1.f},
  //     {.1f, 1.f, .1f},
  //     {1.f, 1.f, .1f},
  //     {.1f, 1.f, 1.f},
  //     {.5f, 1.f, 0.f},
  //     {1.f, 0.f, 0.f},
  //     {.1f, .5f, 1.f},
  //     {1.f, 1.f, 1.f}  //
  // };

  std::default_random_engine rng{};
  std::uniform_real_distribution<float> dist{0.1f,1.f};
  std::uniform_real_distribution<float> intesnsityDist{0.1f,2.5f};
  std::uniform_real_distribution<float> sizeDist{0.5f,1.0f};

  std::vector<glm::vec3> lightColors;
  lightColors.resize(5);
  
  


    for(int i=0; i < lightColors.size(); i++){
      lightColors[i] = {dist(rng),dist(rng),dist(rng)};
      auto pointLight = CFXGameObject::makePointLight(intesnsityDist(rng),sizeDist(rng));
      auto rotateLight = glm::rotate(glm::mat4(0.9f),(i * glm::two_pi<float>()) / lightColors.size(),{0.f,1.f,0.f});
      // std::cout << glm::to_string(rotateLight) << std::endl;
      pointLight.color = lightColors[i];
      pointLight.transformComponent.translation = glm::vec3(rotateLight * glm::vec4(-1.f,-1.f,-1.f,-1.f));
      cfxGameObjects.emplace(pointLight.getId(),std::move(pointLight));
    }
  }

}