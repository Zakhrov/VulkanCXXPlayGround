#pragma once

#include "cfx_window.hpp"
#include "cfx_device.hpp"
#include "cfx_swapchain.hpp"
#include "cfx_model.hpp"

#include<memory>
#include<vector>
#include <glm/glm.hpp>
#include <cassert>

namespace cfx{
    class Renderer{
        public:
        
        Renderer(CFXWindow &cfxWindow, CFXDevice &cfxDevice);
        ~Renderer();
        Renderer(const Renderer &) = delete;
        Renderer &operator=(const Renderer &) = delete;

        VkCommandBuffer beginFrame();
        void endFrame();
        bool isFrameInProgress() const {return isFrameStarted;}
        VkCommandBuffer getCurrentCommandBuffer() const{ 
            assert( isFrameStarted && "Cannot get Command Buffer if frame is not in progress");
            return commandBuffers[currentFrameIndex];
            }
        int getFrameIndex() const {
            assert( isFrameStarted && "Cannot get Frame Index if frame is not in progress");
            return currentFrameIndex;

        }
        VkRenderPass getSwapChainRenderPass() const {return cfxSwapChain->getRenderPass();}
        void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
        void endSwapChainRenderPass(VkCommandBuffer commandBuffer);
        


        private:
      
        void createCommandBuffers();
        void freeCommandBuffers();
        void recreateSwapChain();
        
        
       

        CFXWindow& cfxWindow;
        CFXDevice& cfxDevice;
        // CFXSwapChain cfxSwapChain{cfxDevice,window.getExtent()};
        std::unique_ptr<CFXSwapChain> cfxSwapChain;
        // CFXPipeLine cfxPipeLine{cfxDevice,CFXPipeLine::defaultPipelineConfigInfo(WIDTH,HEIGHT),"shaders/simple_shader.vert.spv","shaders/simple_shader.frag.spv"};
        std::vector<VkCommandBuffer> commandBuffers;
        uint32_t currentImageIndex;
        int currentFrameIndex{0};
        bool isFrameStarted = false;
        uint32_t deviceIndex = 1;
        
        
    };
}