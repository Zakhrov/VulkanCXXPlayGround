#include "cfx_window.hpp"
#include <stdexcept>
namespace cfx{
    CFXWindow::CFXWindow(int w,int h, std::string name): width{w},height{h},windowName{name} {
        initWindow();
    }
    CFXWindow::~CFXWindow(){
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void CFXWindow::initWindow(){
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API,GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE,GLFW_FALSE);
        window = glfwCreateWindow(width,height,windowName.c_str(),nullptr,nullptr);
    }
    void CFXWindow::createWindowSurface(VkInstance instance,VkSurfaceKHR* surface_){
        if(glfwCreateWindowSurface(instance,window,nullptr,surface_)!=VK_SUCCESS){
            throw std::runtime_error("Failed to create window surface");

        }
    }
}