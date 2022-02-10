#include "cfx_render_system.hpp"
#include <stdexcept>
#include <array>
#include <iostream>
#include <memory>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace cfx
{
  struct SimplePushConstantData
  {
    glm::mat4 transform{1.f};
    glm::mat4 modelMatrix{1.f};
    // alignas(16) glm::vec3 color;
  };
  CFXRenderSystem::CFXRenderSystem(CFXDevice &device, std::vector<VkRenderPass> renderPasses) : cfxDevice{device}
  {
    pipelineLayout.resize(cfxDevice.getDevicesinDeviceGroup());
    cfxPipeLines.resize(cfxDevice.getDevicesinDeviceGroup());
    for (int deviceIndex = 0; deviceIndex < cfxDevice.getDevicesinDeviceGroup(); deviceIndex++)
    {
      createPipelineLayout(deviceIndex);
      createPipeline(renderPasses[deviceIndex], deviceIndex);
    }
  }
  CFXRenderSystem::~CFXRenderSystem()
  {
    for (int i = 0; i < cfxDevice.getDevicesinDeviceGroup(); i++)
    {
      vkDestroyPipelineLayout(cfxDevice.device(i), pipelineLayout[i], nullptr);
    }
  }

  void CFXRenderSystem::renderGameObjects(VkCommandBuffer commandBuffer, std::vector<CFXGameObject> &cfxGameObjects, int deviceIndex, const CFXCamera camera)
  {
    // std::cout << "RENDER GAME OBJECTS ON " << cfxDevice.getDeviceName(deviceIndex) << std::endl;
    cfxPipeLines[deviceIndex]->bind(commandBuffer);
    auto projectionView = camera.getProjection() * camera.getView();

    for (auto &obj : cfxGameObjects)
    {
      // obj.transformComponent.rotation.y = glm::mod(obj.transformComponent.rotation.y + 0.01f, glm::two_pi<float>());
      // obj.transformComponent.rotation.x = glm::mod(obj.transformComponent.rotation.x + 0.005f, glm::two_pi<float>());

      SimplePushConstantData push{};
      auto modelMatrix = obj.transformComponent.mat4();

      push.transform = projectionView * modelMatrix;
      push.modelMatrix = modelMatrix;

      vkCmdPushConstants(
          commandBuffer,
          pipelineLayout[deviceIndex],
          VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
          0,
          sizeof(SimplePushConstantData),
          &push);
      obj.model->bind(commandBuffer, deviceIndex);
      obj.model->draw(commandBuffer);
      // std::cout << "RENDER GAME OBJECTS END ON " << std::endl;
    }
  }
  void CFXRenderSystem::createPipelineLayout(int deviceIndex)
  {
    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(SimplePushConstantData);
    // std::cout << "CREATE PIPELINE LAYOUT  "  << std::endl;
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    pipelineLayoutInfo.pushConstantRangeCount = 1;

    if (vkCreatePipelineLayout(cfxDevice.device(deviceIndex), &pipelineLayoutInfo, nullptr, &pipelineLayout[deviceIndex]) != VK_SUCCESS)
    {
      throw std::runtime_error("failed to create pipeline layout");
    }
  }
  void CFXRenderSystem::createPipeline(VkRenderPass renderpass, int deviceIndex)
  {

    // std::cout << "CREATE PIPELINE " << std::endl;
    PipelineConfigInfo pipelineConfig{};
    CFXPipeLine::defaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.renderPass = renderpass;
    pipelineConfig.pipelineLayout = pipelineLayout[deviceIndex];
    cfxPipeLines[deviceIndex] = std::make_unique<CFXPipeLine>(cfxDevice, pipelineConfig,
                                                              "shaders/simple_shader.vert.spv",
                                                              "shaders/simple_shader.frag.spv", deviceIndex);
    // std::cout << "CREATE PIPELINE END " << std::endl;
  }

}