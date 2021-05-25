#pragma once

#include "cfx_window.hpp"
#include "cfx_pipeline.hpp"
#include "cfx_device.hpp"
#include "cfx_swapchain.hpp"
#include "cfx_model.hpp"
#include<memory>
#include<vector>
#include <glm/glm.hpp>
namespace cfx{
    class App{
        public:
        static constexpr int WIDTH = 800;
        static constexpr int HEIGHT = 600;
        App();
        ~App();
        App(const App &) = delete;
        App &operator=(const App &) = delete;
        void run();


        private:
        void loadModels();
        void createPipelineLayout();
        void createPipeline();
        void createCommandBuffers();
        void drawFrame();
        void recreateSwapChain();
        void recordCommandBuffer(int imageIndex);
        void sierpinski(
    std::vector<CFXModel::Vertex> &vertices,
    int depth,
    glm::vec2 left,
    glm::vec2 right,
    glm::vec2 top);

        CFXWindow window{WIDTH,HEIGHT,"Hello Vulkan"};
        CFXDevice cfxDevice{window};
        // CFXSwapChain cfxSwapChain{cfxDevice,window.getExtent()};
        std::unique_ptr<CFXSwapChain> cfxSwapChain;
        // CFXPipeLine cfxPipeLine{cfxDevice,CFXPipeLine::defaultPipelineConfigInfo(WIDTH,HEIGHT),"shaders/simple_shader.vert.spv","shaders/simple_shader.frag.spv"};
        std::unique_ptr<CFXPipeLine> cfxPipeLine;
        VkPipelineLayout pipelineLayout;
        std::vector<VkCommandBuffer> commandBuffers;
        std::unique_ptr<CFXModel> cfxModel; 
        
    };
}