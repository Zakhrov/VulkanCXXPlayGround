#pragma once


#include "cfx_pipeline.hpp"
#include "cfx_device.hpp"
#include "cfx_camera.hpp"

#include "cfx_model.hpp"
#include "cfx_game_object.hpp"
#include<memory>
#include<vector>
#include <glm/glm.hpp>
namespace cfx{
    class CFXRenderSystem{
        public:
            CFXRenderSystem(CFXDevice &device, std::vector<VkRenderPass> renderPasses);
            ~CFXRenderSystem();
            CFXRenderSystem(const CFXRenderSystem &) = delete;
            CFXRenderSystem &operator=(const CFXRenderSystem &) = delete;
            void renderGameObjects(VkCommandBuffer commandBuffer, std::vector<CFXGameObject> &gameObjects,int deviceIndex, const CFXCamera camera);

        private:
            void createPipelineLayout(int deviceIndex);
            void createPipeline(VkRenderPass renderpass,int deviceIndex);
            
            CFXDevice& cfxDevice;
            // CFXSwapChain cfxSwapChain{cfxDevice,window.getExtent()};
            
            std::vector<std::unique_ptr<CFXPipeLine>> cfxPipeLines;
            std::vector<VkPipelineLayout> pipelineLayout; 
    };
}