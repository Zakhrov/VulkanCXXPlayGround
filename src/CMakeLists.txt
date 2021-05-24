set(SOURCES main.cpp cfx_window.cpp cfx_app.cpp cfx_pipeline.cpp cfx_device.cpp cfx_swapchain.cpp cfx_model.cpp)
add_executable(Vulkantest ${SOURCES})
target_link_libraries(Vulkantest vulkan)
target_link_libraries(Vulkantest glfw)
