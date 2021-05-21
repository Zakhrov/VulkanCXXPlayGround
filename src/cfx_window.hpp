#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>

namespace cfx{
    class CFXWindow {
        public:
        CFXWindow(int w,int h, std::string name);
        ~CFXWindow();
        CFXWindow &operator=(const CFXWindow &) = delete;
        bool shouldClose() {return glfwWindowShouldClose(window);}
        void createWindowSurface(VkInstance instance,VkSurfaceKHR* surface_);

        private:
        void initWindow();
        const int width;
        const int height;
        std::string windowName;
            GLFWwindow *window;

    };
}