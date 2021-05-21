set(SOURCES main.cpp cfx_window.cpp cfx_app.cpp)
add_executable(Vulkantest ${SOURCES})
target_link_libraries(Vulkantest vulkan)
target_link_libraries(Vulkantest glfw)
