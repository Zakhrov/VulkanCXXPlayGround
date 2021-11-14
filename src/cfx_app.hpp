#pragma once

#include "cfx_window.hpp"
#include "cfx_device.hpp"
#include "cfx_renderer.hpp"
#include "cfx_model.hpp"
#include "cfx_game_object.hpp"
#include "cfx_descriptors.hpp"
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
        void loadGameObjects();
        
        
        
        
        
        
    

        CFXWindow window{WIDTH,HEIGHT,"Hello Vulkan"};
        CFXDevice cfxDevice{window};
        // CFXSwapChain cfxSwapChain{cfxDevice,window.getExtent()};
        Renderer cfxRenderer{window,cfxDevice};
        std::vector<std::unique_ptr<CFXDescriptorPool>> cfxDescriptorPools;
        std::vector<CFXGameObject> cfxGameObjects; 
        
        
    };
}