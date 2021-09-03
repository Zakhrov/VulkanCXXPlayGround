#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "cfx_model.hpp"
#include "cfx_utils.hpp"
#include <cassert>
#include <cstring>
#include <iostream>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <unordered_map>

namespace std{
    template<>
    struct hash<cfx::CFXModel::Vertex>{
        size_t operator()(cfx::CFXModel::Vertex const &vertex) const{
            size_t seed = 0;
            cfx::hashCombine(seed,vertex.position,vertex.color,vertex.normal,vertex.uv);
            
            // cout << "SEED " << seed << endl;
            return seed;
        }
    };
}



namespace cfx{
    CFXModel::CFXModel(CFXDevice& device,const CFXModel::Builder &builder): cfxDevice{device}
    {
        vertexBuffers.resize(cfxDevice.getDevicesinDeviceGroup());
        indexBuffers.resize(cfxDevice.getDevicesinDeviceGroup());
        vertexBuffersBound.resize(cfxDevice.getDevicesinDeviceGroup());
        indexBuffersBound.resize(cfxDevice.getDevicesinDeviceGroup());
        vertexBufferMemories.resize(cfxDevice.getDevicesinDeviceGroup());
        indexBufferMemories.resize(cfxDevice.getDevicesinDeviceGroup());
        hasIndexBuffer.resize(cfxDevice.getDevicesinDeviceGroup());
        for(int i=0; i < cfxDevice.getDevicesinDeviceGroup(); i++){
            hasIndexBuffer[i] = false;
            createVertexBuffers(builder.vertices,i);
            createIndexBuffers(builder.indices,i);
        }

    }
    CFXModel::~CFXModel(){
        for(int i=0; i <cfxDevice.getDevicesinDeviceGroup(); i++){
            vkDestroyBuffer(cfxDevice.device(i),vertexBuffers[i],nullptr);
        vkFreeMemory(cfxDevice.device(i),vertexBufferMemories[i],nullptr);

        if(hasIndexBuffer[i]){
            vkDestroyBuffer(cfxDevice.device(i),indexBuffers[i],nullptr);
        vkFreeMemory(cfxDevice.device(i),indexBufferMemories[i],nullptr);
        }
            
        }
        


    }
    std::unique_ptr<CFXModel> CFXModel::createModelFromFile(CFXDevice &device,const std::string &filepath){
        Builder builder{};
        builder.loadModel(filepath);
        std::cout << "Vertex Count "<< builder.vertices.size() << std::endl;
        return std::make_unique<CFXModel>(device,builder);
    }
    void CFXModel::createVertexBuffers(const std::vector<Vertex> &vertices,int deviceIndex){
        vertexCount = static_cast<uint32_t>(vertices.size());
        assert(vertexCount >= 3 && "Vertex count must be at least 3");
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        cfxDevice.createBuffer(bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingBufferMemory, deviceIndex);
        void *data;
            vkMapMemory(cfxDevice.device(deviceIndex),stagingBufferMemory,0,bufferSize,0,&data);
        memcpy(data,vertices.data(),static_cast<size_t>(bufferSize));
        vkUnmapMemory(cfxDevice.device(deviceIndex),stagingBufferMemory);
        cfxDevice.createBuffer(bufferSize,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        vertexBuffers[deviceIndex],
        vertexBufferMemories[deviceIndex],deviceIndex);

        cfxDevice.copyBuffer(stagingBuffer,vertexBuffers[deviceIndex],bufferSize,deviceIndex);
        vkDestroyBuffer(cfxDevice.device(deviceIndex),stagingBuffer,nullptr);
        vkFreeMemory(cfxDevice.device(deviceIndex),stagingBufferMemory,nullptr);



    }

     void CFXModel::createIndexBuffers(const std::vector<uint32_t> &indices, int deviceIndex){
         indexCount = static_cast<uint32_t>(indices.size());
        hasIndexBuffer[deviceIndex] = indexCount > 0;
        if(!hasIndexBuffer[deviceIndex]){
            return;
        }
        VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        cfxDevice.createBuffer(bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingBufferMemory,deviceIndex);
        void *data;
            vkMapMemory(cfxDevice.device(deviceIndex),stagingBufferMemory,0,bufferSize,0,&data);
        memcpy(data,indices.data(),static_cast<size_t>(bufferSize));
        vkUnmapMemory(cfxDevice.device(deviceIndex),stagingBufferMemory);

         cfxDevice.createBuffer(bufferSize,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        indexBuffers[deviceIndex],
        indexBufferMemories[deviceIndex],deviceIndex);

        cfxDevice.copyBuffer(stagingBuffer,indexBuffers[deviceIndex],bufferSize,deviceIndex);
        vkDestroyBuffer(cfxDevice.device(deviceIndex),stagingBuffer,nullptr);
        vkFreeMemory(cfxDevice.device(deviceIndex),stagingBufferMemory,nullptr);
        


    }
    void CFXModel::draw(VkCommandBuffer commandBuffer,int deviceIndex){
        if(hasIndexBuffer[deviceIndex]){
            vkCmdDrawIndexed(commandBuffer,indexCount,1,0,0,0);
        }else{
            vkCmdDraw(commandBuffer,vertexCount,1,0,0);
        }
        
    }

    void CFXModel::bind(VkCommandBuffer commandBuffer,int deviceIndex){
        
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer,0,1,&vertexBuffers[deviceIndex],offsets);

        if(hasIndexBuffer[deviceIndex]){
            vkCmdBindIndexBuffer(commandBuffer,indexBuffers[deviceIndex],0,VK_INDEX_TYPE_UINT32);
        }
        
    }

    std::vector<VkVertexInputBindingDescription> CFXModel::Vertex::getBindingDescriptions(){
        std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
        bindingDescriptions[0].binding = 0;
        bindingDescriptions[0].stride =sizeof(Vertex);
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescriptions;
    }
    std::vector<VkVertexInputAttributeDescription> CFXModel::Vertex::getAttributeDescriptions(){
        std::vector<VkVertexInputAttributeDescription> inputAttributeDescriptions{};
        inputAttributeDescriptions.push_back({0,0,VK_FORMAT_R32G32B32_SFLOAT,offsetof(Vertex,position)});
        inputAttributeDescriptions.push_back({1,0,VK_FORMAT_R32G32B32_SFLOAT,offsetof(Vertex,color)});
        inputAttributeDescriptions.push_back({2,0,VK_FORMAT_R32G32B32_SFLOAT,offsetof(Vertex,normal)});
        inputAttributeDescriptions.push_back({3,0,VK_FORMAT_R32G32_SFLOAT,offsetof(Vertex,uv)});
        return inputAttributeDescriptions;

    }
    void CFXModel::Builder::loadModel(const std::string &filepath){
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn;
        std::string error;
        if(!tinyobj::LoadObj(&attrib,&shapes,&materials,&warn,&error,filepath.c_str())){
            throw std::runtime_error(warn+error);
        }
        vertices.clear();
        indices.clear();
        std::unordered_map<Vertex,uint32_t> uniqueVertices{};

        for(const auto &shape : shapes){
            for(const auto &index : shape.mesh.indices){
                Vertex vertex{};

                if(index.vertex_index >= 0){
                    vertex.position = {
                        attrib.vertices[3* index.vertex_index + 0],
                    attrib.vertices[3* index.vertex_index + 1],
                    attrib.vertices[3* index.vertex_index + 2],
                    };
                     vertex.color = {
                        attrib.colors[3* index.vertex_index + 0],
                    attrib.colors[3* index.vertex_index + 1],
                    attrib.colors[3* index.vertex_index + 2],
                    };


                }
                if(index.normal_index >= 0){
                    vertex.normal = {
                        attrib.normals[3* index.normal_index + 0],
                    attrib.normals[3* index.normal_index + 1],
                    attrib.normals[3* index.normal_index + 2],
                    };

                }

                 if(index.texcoord_index >= 0){
                    vertex.uv = {
                        attrib.texcoords[3* index.texcoord_index + 0],
                    attrib.texcoords[3* index.normal_index + 1],
                    
                    };


                }
                if(uniqueVertices.count(vertex)== 0){
                    
                    // std::cout << "VERTEX ARRAY SIZE " << static_cast<uint32_t>(vertices.size()) << std::endl;
                    uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);
                }
                // std::cout << "VERTEX COUNT " << uniqueVertices.count(vertex) << std::endl;
                // std::cout << "VERTEX INDEX " << uniqueVertices[vertex] << std::endl;
                indices.push_back(uniqueVertices[vertex]);
            }

        }

    }

}