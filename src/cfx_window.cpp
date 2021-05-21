#include "cfx_window.hpp"
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
}