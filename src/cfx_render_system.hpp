#pragma once


#include "cfx_pipeline.hpp"
#include "cfx_device.hpp"

#include "cfx_model.hpp"
#include "cfx_game_object.hpp"
#include<memory>
#include<vector>
#include <glm/glm.hpp>
namespace cfx{
    class CFXRenderSystem{
        public:
        CFXRenderSystem(CFXDevice &device, VkRenderPass renderPass);
        ~CFXRenderSystem();
        CFXRenderSystem(const CFXRenderSystem &) = delete;
        CFXRenderSystem &operator=(const CFXRenderSystem &) = delete;
        void renderGameObjects(VkCommandBuffer commandBuffer, std::vector<CFXGameObject> &gameObjects,uint32_t deviceMask);
        


        private:
        
        void createPipelineLayout();
        void createPipeline(VkRenderPass renderpass);
        
        
        
        
        
    

        
        CFXDevice& cfxDevice;
        // CFXSwapChain cfxSwapChain{cfxDevice,window.getExtent()};
        
        std::unique_ptr<CFXPipeLine> cfxPipeLine;
        VkPipelineLayout pipelineLayout;
        std::vector<VkCommandBuffer> commandBuffers;
        
        
    };
}