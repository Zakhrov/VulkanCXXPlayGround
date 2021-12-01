cmake_minimum_required(VERSION 3.17)

project(VulkanProject)

find_package(glfw3 REQUIRED)
find_package(Vulkan REQUIRED)

add_subdirectory(src)
