#pragma once

#include "cfx_camera.hpp"
#include "cfx_game_object.hpp"

#include <vulkan/vulkan.h>

namespace cfx
{
    #define MAX_LIGHTS 1000

    struct PointLight{
        glm::vec4 position{};
        glm::vec4 color{};


    };
    struct FrameInfo
    {
        int frameIndex;
        float frameTime;
        VkCommandBuffer commandBuffer;
        CFXCamera &camera;
        uint32_t deviceIndex;
        VkDescriptorSet globalDescriptorSet;
        CFXGameObject::Map &gameObjects;
    };

    struct GlobalUbo
    {
        glm::mat4 projection{1.f};
        glm::mat4 view{1.f};
        glm::mat4 inverseView{1.f};
        glm::vec4 ambientLightColor{1.f, 1.f, 1.f, .7f};
        PointLight pointLights[MAX_LIGHTS];
        int numLights;
    };
}