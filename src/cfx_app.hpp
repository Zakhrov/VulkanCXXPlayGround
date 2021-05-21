#pragma once

#include "cfx_window.hpp"
#include "cfx_pipeline.hpp"
#include "cfx_device.hpp"
namespace cfx{
    class App{
        public:
        static constexpr int WIDTH = 800;
        static constexpr int HEIGHT = 600;
        void run();


        private:
        CFXWindow window{WIDTH,HEIGHT,"Hello Vulkan"};
        CFXDevice cfxDevice{window};
        CFXPipeLine cfxPipeLine{cfxDevice,CFXPipeLine::defaultPipelineConfigInfo(WIDTH,HEIGHT),"shaders/simple_shader.vert.spv","shaders/simple_shader.frag.spv"};
    };
}