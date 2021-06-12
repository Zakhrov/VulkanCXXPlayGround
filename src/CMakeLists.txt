set(SOURCES main.cpp cfx_window.cpp cfx_app.cpp cfx_pipeline.cpp cfx_device.cpp cfx_swapchain.cpp cfx_model.cpp)

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

add_custom_target(shaders ALL DEPENDS ${SPV_SHADERS})
add_executable(Vulkantest ${SOURCES} ${SHADERS})
add_dependencies(Vulkantest shaders)
target_link_libraries(Vulkantest vulkan)
target_link_libraries(Vulkantest glfw)
