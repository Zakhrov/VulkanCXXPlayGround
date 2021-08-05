#pragma once

#include "cfx_device.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <vector>



namespace cfx {
    class CFXModel{
        public:
        struct Vertex{
            glm::vec3 position;
            glm::vec3 color;
            static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
            static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
        };
        CFXModel(CFXDevice& device,const std::vector<Vertex> &vertices);
        ~CFXModel();
        CFXModel(const CFXModel &) = delete;
        CFXModel &operator=(const CFXModel &) = delete;

        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);
        private:
        void createVertexBuffers(const std::vector<Vertex> &vertices);
        CFXDevice& cfxDevice;
        VkBuffer vertexBuffer;
        VkDeviceMemory vertexBufferMemory;
        uint32_t vertexCount;
        

    };
}