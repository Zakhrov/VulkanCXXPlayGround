#include "cfx_model.hpp"
#include <cassert>
#include <cstring>



namespace cfx{
    CFXModel::CFXModel(CFXDevice& device,const std::vector<Vertex> &vertices): cfxDevice{device}
    {
        createVertexBuffers(vertices);

    }
    CFXModel::~CFXModel(){
        vkDestroyBuffer(cfxDevice.device(),vertexBuffer,nullptr);
        vkFreeMemory(cfxDevice.device(),vertexBufferMemory,nullptr);


    }
    void CFXModel::createVertexBuffers(const std::vector<Vertex> &vertices){
        vertexCount = static_cast<uint32_t>(vertices.size());
        assert(vertexCount >= 3 && "Vertex count must be at least 3");
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
        cfxDevice.createBuffer(bufferSize,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
        vertexBuffer,
        vertexBufferMemory);
        void *data;
        vkMapMemory(cfxDevice.device(),vertexBufferMemory,0,bufferSize,0,&data);
        memcpy(data,vertices.data(),static_cast<size_t>(bufferSize));
        vkUnmapMemory(cfxDevice.device(),vertexBufferMemory);
        


    }
    void CFXModel::draw(VkCommandBuffer commandBuffer){
        vkCmdDraw(commandBuffer,vertexCount,1,0,0);
    }

    void CFXModel::bind(VkCommandBuffer commandBuffer){
        VkBuffer buffers[] = {vertexBuffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer,0,1,buffers,offsets);
        
    }

    std::vector<VkVertexInputBindingDescription> CFXModel::Vertex::getBindingDescriptions(){
        std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
        bindingDescriptions[0].binding = 0;
        bindingDescriptions[0].stride =sizeof(Vertex);
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescriptions;
    }
    std::vector<VkVertexInputAttributeDescription> CFXModel::Vertex::getAttributeDescriptions(){
        std::vector<VkVertexInputAttributeDescription> inputAttributeDescriptions(2);
        inputAttributeDescriptions[0].binding = 0;
        inputAttributeDescriptions[0].location = 0;
        inputAttributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        inputAttributeDescriptions[0].offset = offsetof(Vertex,position);

        inputAttributeDescriptions[1].binding = 0;
        inputAttributeDescriptions[1].location = 1;
        inputAttributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        inputAttributeDescriptions[1].offset = offsetof(Vertex,color);
        return inputAttributeDescriptions;

    }

}