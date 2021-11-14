set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES main.cpp cfx_buffer.cpp cfx_camera.cpp cfx_descriptors.cpp cfx_window.cpp cfx_app.cpp cfx_pipeline.cpp cfx_device.cpp cfx_swapchain.cpp cfx_model.cpp cfx_renderer.cpp cfx_render_system.cpp cfx_game_object.cpp keyboard_movement_controller.cpp)

set( ${CMAKE_CURRENT_SOURCE_DIR}/shaders)

set(SHADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
file(GLOB SHADERS ${SHADER_DIR}/*.vert ${SHADER_DIR}/*.frag ${SHADER_DIR}/*.comp ${SHADER_DIR}/*.geom ${SHADER_DIR}/*.tesc ${SHADER_DIR}/*.tese ${SHADER_DIR}/*.mesh ${SHADER_DIR}/*.task ${SHADER_DIR}/*.rgen ${SHADER_DIR}/*.rchit ${SHADER_DIR}/*.rmiss)

find_package(Vulkan)

foreach(SHADER IN LISTS SHADERS)
    get_filename_component(FILENAME ${SHADER} NAME)
    add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/shaders/${FILENAME}.spv
        COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${SHADER} -o ${CMAKE_CURRENT_BINARY_DIR}/shaders/${FILENAME}.spv
        DEPENDS ${SHADER}
        COMMENT "Compiling ${FILENAME}")
list(APPEND SPV_SHADERS ${CMAKE_CURRENT_BINARY_DIR}/shaders/${FILENAME}.spv)
endForeach()

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/models/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/models/)


add_custom_target(shaders ALL DEPENDS ${SPV_SHADERS})
add_executable(Vulkantest ${SOURCES} ${SHADERS})
add_dependencies(Vulkantest shaders)
target_link_libraries(Vulkantest vulkan)
target_link_libraries(Vulkantest glfw)
