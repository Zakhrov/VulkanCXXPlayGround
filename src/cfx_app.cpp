#include "cfx_app.hpp"

namespace cfx{
    void App::run(){
        while(!window.shouldClose()){
            glfwPollEvents();
        }
    }
}