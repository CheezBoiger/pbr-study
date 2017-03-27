# Copyright (c) Mario Garcia, MIT License.
cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)
project("PBR-Study")

option(
  EXTERNAL_TESTS 
  "Build application along with external library tests"
  OFF
)


# Find Vulkan!!
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")


set(ENABLE_GLSLANG_BINARIES   OFF CACHE BOOL  "GLSlang binaries")
set(GLFW_VULKAN_STATIC        OFF CACHE BOOL "GLFW vulkan lib")
set(GLFW_BUILD_EXAMPLES       OFF CACHE BOOL  "GLFW examples")
set(GLFW_BUILD_TESTS          OFF CACHE BOOL  "GLFW tests")
set(BUILD_SHARED_LIBS         OFF CACHE BOOL  "building shared libes for GLFW")

add_subdirectory(external/glfw)
add_subdirectory(external/glslang)
#add_subdirectory(external/freetype)
add_subdirectory(src)
