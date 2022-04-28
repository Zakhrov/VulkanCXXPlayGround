#include "cfx_point_light_system.hpp"
#include <stdexcept>
#include <array>
#include <iostream>
#include <memory>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <map>

namespace cfx
{

    struct PointLightPushConstants
    {
        glm::vec4 position{};
        glm::vec4 color{};
        float radius;
    };

    CFXPointLightSystem::CFXPointLightSystem(CFXDevice &device, std::vector<VkRenderPass> renderPasses, std::vector<std::unique_ptr<CFXDescriptorSetLayout>> &cfxSetLayouts) : cfxDevice{device}
    {
        pipelineLayout.resize(cfxDevice.getDevicesinDeviceGroup());
        cfxPipeLines.resize(cfxDevice.getDevicesinDeviceGroup());
        for (int deviceIndex = 0; deviceIndex < cfxDevice.getDevicesinDeviceGroup(); deviceIndex++)
        {
            createPipelineLayout(cfxSetLayouts[deviceIndex]->getDescriptorSetLayout(), deviceIndex);
            createPipeline(renderPasses[deviceIndex], deviceIndex);
        }
    }
    CFXPointLightSystem::~CFXPointLightSystem()
    {
        for (int i = 0; i < cfxDevice.getDevicesinDeviceGroup(); i++)
        {
            vkDestroyPipelineLayout(cfxDevice.device(i), pipelineLayout[i], nullptr);
        }
    }

    void CFXPointLightSystem::update(FrameInfo &frameInfo, GlobalUbo &ubo)
    {
        // std::cout << "UPDATING POINT LIGHT " << frameInfo.frameTime << std::endl;
        auto rotateLight = glm::rotate(glm::mat4(1.f), glm::two_pi<float>() / 4000.f, {0.1, -1.f, 0.f});
        int lightIndex = 0;
        for (auto &kv : frameInfo.gameObjects)
        {
            auto &obj = kv.second;
            if (obj.pointLight == nullptr)
            {
                continue;
            }
            obj.transformComponent.translation = glm::vec3(rotateLight * glm::vec4(obj.transformComponent.translation, 1.f));
            ubo.pointLights[lightIndex].position = glm::vec4(obj.transformComponent.translation, 1.f);
            ubo.pointLights[lightIndex].color = glm::vec4(obj.color, obj.pointLight->lightIntensity);
            lightIndex++;
        }
        ubo.numLights = lightIndex;
    }

    void CFXPointLightSystem::render(FrameInfo &frameInfo)
    {
        std::map<float, CFXGameObject::id_t> sorted;
        for (auto &kv : frameInfo.gameObjects)
        {
            auto &obj = kv.second;
            if (obj.pointLight == nullptr)
            {
                continue;
            }
            // calculate distance
            auto offset = frameInfo.camera.getPosition() - obj.transformComponent.translation;
            float distSquared = glm::dot(offset, offset);
            sorted[distSquared] = obj.getId();
        }

        // std::cout << "RENDER POINT LIGHT ON " << cfxDevice.getDeviceName(frameInfo.deviceIndex) << std::endl;
        // if (frameInfo.deviceIndex > 0)
        // {

        cfxPipeLines[frameInfo.deviceIndex]->bind(frameInfo.commandBuffer);
        vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout[frameInfo.deviceIndex], 0, 1, &frameInfo.globalDescriptorSet, 0, nullptr);
        for (auto it = sorted.rbegin(); it != sorted.rend(); ++it)
        {
            auto &obj = frameInfo.gameObjects.at(it->second);
            PointLightPushConstants push{};
            push.position = glm::vec4(obj.transformComponent.translation, 1.f);
            push.color = glm::vec4(obj.color, obj.pointLight->lightIntensity);
            push.radius = obj.transformComponent.scale.x;
            vkCmdPushConstants(frameInfo.commandBuffer, pipelineLayout[frameInfo.deviceIndex], VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PointLightPushConstants), &push);
            vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
        }
        // }
    }
    void CFXPointLightSystem::createPipelineLayout(VkDescriptorSetLayout descriptorSetLayout, int deviceIndex)
    {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(PointLightPushConstants);
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
    void CFXPointLightSystem::createPipeline(VkRenderPass renderpass, int deviceIndex)
    {

        // std::cout << "CREATE PIPELINE " << std::endl;
        PipelineConfigInfo pipelineConfig{};
        CFXPipeLine::defaultPipelineConfigInfo(pipelineConfig);
        CFXPipeLine::enableAlphaBlending(pipelineConfig);
        pipelineConfig.attributeDescriptions.clear();
        pipelineConfig.bindingDescriptions.clear();

        pipelineConfig.renderPass = renderpass;
        pipelineConfig.pipelineLayout = pipelineLayout[deviceIndex];
        cfxPipeLines[deviceIndex] = std::make_unique<CFXPipeLine>(cfxDevice, pipelineConfig,
                                                                  "shaders/point_light.vert.spv",
                                                                  "shaders/point_light.frag.spv", deviceIndex);
        // std::cout << "CREATE PIPELINE END " << std::endl;
    }

}