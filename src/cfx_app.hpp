#pragma once

#include "cfx_window.hpp"
#include "cfx_pipeline.hpp"
#include "cfx_device.hpp"
#include "cfx_swapchain.hpp"
#include<memory>
#include<vector>
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
        void createPipelineLayout();
        void createPipeline();
        void createCommandBuffers();
        void drawFrame();

        CFXWindow window{WIDTH,HEIGHT,"Hello Vulkan"};
        CFXDevice cfxDevice{window};
        CFXSwapChain cfxSwapChain{cfxDevice,window.getExtent()};
        // CFXPipeLine cfxPipeLine{cfxDevice,CFXPipeLine::defaultPipelineConfigInfo(WIDTH,HEIGHT),"shaders/simple_shader.vert.spv","shaders/simple_shader.frag.spv"};
        std::unique_ptr<CFXPipeLine> cfxPipeLine;
        VkPipelineLayout pipelineLayout;
        std::vector<VkCommandBuffer> commandBuffers;
        
    };
}