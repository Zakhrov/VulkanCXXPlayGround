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
    glm::mat4 modelMatrix{1.f};
    glm::mat4 normlaMatrix{1.f};
    // alignas(16) glm::vec3 color;
  };
  CFXRenderSystem::CFXRenderSystem(CFXDevice &device, std::vector<VkRenderPass> renderPasses, std::vector<std::unique_ptr<CFXDescriptorSetLayout>> &cfxSetLayouts) : cfxDevice{device}
  {
    pipelineLayout.resize(cfxDevice.getDevicesinDeviceGroup());
    cfxPipeLines.resize(cfxDevice.getDevicesinDeviceGroup());
    for (int deviceIndex = 0; deviceIndex < cfxDevice.getDevicesinDeviceGroup(); deviceIndex++)
    {
      createPipelineLayout(cfxSetLayouts[deviceIndex]->getDescriptorSetLayout(), deviceIndex);
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

  void CFXRenderSystem::renderGameObjects(FrameInfo &frameInfo)
  {
    // std::cout << "RENDER GAME OBJECTS ON " << cfxDevice.getDeviceName(deviceIndex) << std::endl;
    cfxPipeLines[frameInfo.deviceIndex]->bind(frameInfo.commandBuffer);
    vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout[frameInfo.deviceIndex], 0, 1, &frameInfo.globalDescriptorSet, 0, nullptr);

    for (auto &kv : frameInfo.gameObjects)
    {
      auto &obj = kv.second;
      if (obj.model == nullptr)
        continue;
      SimplePushConstantData push{};
      push.modelMatrix = obj.transformComponent.mat4();
      push.normlaMatrix = obj.transformComponent.normalMatrix();

      vkCmdPushConstants(
          frameInfo.commandBuffer,
          pipelineLayout[frameInfo.deviceIndex],
          VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
          0,
          sizeof(SimplePushConstantData),
          &push);
      obj.model->bind(frameInfo.commandBuffer, frameInfo.deviceIndex);
      obj.model->draw(frameInfo.commandBuffer);
      // std::cout << "RENDER GAME OBJECTS END ON " << std::endl;
    }
  }
  void CFXRenderSystem::createPipelineLayout(VkDescriptorSetLayout descriptorSetLayout, int deviceIndex)
  {
    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(SimplePushConstantData);
    std::vector<VkDescriptorSetLayout> layouts{descriptorSetLayout};
    // std::cout << "CREATE PIPELINE LAYOUT  "  << std::endl;
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = layouts.size();
    pipelineLayoutInfo.pSetLayouts = layouts.data();
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