#pragma once

#include "cfx_camera.hpp"

#include <vulkan/vulkan.h>

namespace cfx{
    struct FrameInfo{
        int frameIndex;
        float frameTime;
        VkCommandBuffer commandBuffer;
        CFXCamera &camera;
        uint32_t deviceIndex;
        VkDescriptorSet globalDescriptorSet;

    };
}