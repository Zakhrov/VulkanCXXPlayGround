#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>

namespace cfx{
    class CFXWindow {
        public:
        CFXWindow(int w,int h, std::string name);
        ~CFXWindow();
        CFXWindow(const CFXWindow &) = delete;
        CFXWindow &operator=(const CFXWindow &) = delete;
        bool shouldClose() {return glfwWindowShouldClose(window);}
        void createWindowSurface(VkInstance instance,VkSurfaceKHR* surface_);
        bool wasWindowResized() {return framebufferResized;}
        void restWindowResizedFlag(){framebufferResized = false;}
        void setWindowName(std::string name){
            windowName = name;
            glfwSetWindowTitle(getGLFWwindow(),windowName.c_str());
        }
        GLFWwindow *getGLFWwindow() const {return window;}


        VkExtent2D getExtent(){return {static_cast<uint32_t>(width),static_cast<uint32_t>(height)};}

        private:
        void initWindow();
        static void framebufferResizedCallback(GLFWwindow *window,int width,int height);
        int width;
        int height;
        bool framebufferResized = false;
        std::string windowName;
            GLFWwindow *window;

    };
}