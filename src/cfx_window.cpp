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
        glfwWindowHint(GLFW_RESIZABLE,GLFW_TRUE);
        window = glfwCreateWindow(width,height,windowName.c_str(),nullptr,nullptr);
        glfwSetWindowUserPointer(window,this);
        glfwSetFramebufferSizeCallback(window,framebufferResizedCallback);
    }
    void CFXWindow::createWindowSurface(VkInstance instance,VkSurfaceKHR* surface_){
        if(glfwCreateWindowSurface(instance,window,nullptr,surface_)!=VK_SUCCESS){
            throw std::runtime_error("Failed to create window surface");

        }
    }
    void CFXWindow::framebufferResizedCallback(GLFWwindow *window,int width,int height){
        auto cfxWindow = reinterpret_cast<CFXWindow *>(glfwGetWindowUserPointer(window));
        cfxWindow->framebufferResized = true;
        cfxWindow->width = width;
        cfxWindow->height = height;

    }
}