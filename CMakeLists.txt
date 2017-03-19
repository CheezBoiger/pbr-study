# Copyright (c) Mario Garcia, MIT License.
cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)
project("PBR-Study")


# Find Vulkan!!
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

set(ENABLE_GLSLANG_BINARIES OFF)
add_subdirectory(external/assimp)
add_subdirectory(external/glslang)
#add_subdirectory(external/freetype)
add_subdirectory(src/)

