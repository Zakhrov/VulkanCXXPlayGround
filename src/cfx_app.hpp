#pragma once

#include "cfx_window.hpp"

namespace cfx{
    class App{
        public:
        static constexpr int WIDTH = 800;
        static constexpr int HEIGHT = 600;
        void run();


        private:
        CFXWindow window{WIDTH,HEIGHT,"Hello Vulkan"};
    };
}