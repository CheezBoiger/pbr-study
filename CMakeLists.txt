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

set(ENABLE_GLSLANG_BINARIES   OFF)
set(GLFW_VULKAN_STAIC         ON)
set(GLFW_BUILD_EXAMPLES       OFF)
set(GLFW_BUILD_TESTS          OFF)
set(BUILD_SHARED_LIBS         OFF)

set(ASSIMP_BUILD_ASSIMP_TOOLS OFF)
set(ASSIMP_BUILD_TESTS        OFF)
add_subdirectory(external/glfw)
add_subdirectory(external/assimp)
add_subdirectory(external/glslang)
#add_subdirectory(external/freetype)
add_subdirectory(src)

