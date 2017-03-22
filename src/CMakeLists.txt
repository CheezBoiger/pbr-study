# Copyright (c) Mario Garcia, MIT License.
cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)
project("PBR-main")

# Setting up vulkan.
set(PBR_NAME "pbr-main")
include(${CMAKE_SOURCE_DIR}/cmake/math.cmake)
find_package(Vulkan)

if (NOT VULKAN_FOUND)
  message(FATAL_ERROR "Application can not run without Vulkan support!")
endif()

include_directories(SYSTEM
  ${CMAKE_SOURCE_DIR}/math
  ${CMAKE_SOURCE_DIR}/external/freetype/include
  ${CMAKE_SOURCE_DIR}/external/glslang
  ${CMAKE_SOURCE_DIR}/external/glfw/include
  ${Vulkan_INCLUDE_DIRS}
)

set(PBR_SRC
  ${MATH_DIR}
  base.cpp
  base.hpp
  camera.cpp
  camera.hpp
  main.cpp
  model.cpp
  model.hpp
  renderer.cpp
  renderer.hpp
  vertex.hpp
  platform.hpp
  shader.hpp
  shader.cpp
  stb_image.h
  tiny_obj_loader.h
)

add_executable(${PBR_NAME}
  ${PBR_SRC}
)

target_compile_definitions(${PBR_NAME} PRIVATE
  PBR_CURRENT_VERSION=0x000001
  PBR_MINIMUM_VERSION=0x000000
)

target_link_libraries(${PBR_NAME}
  ${Vulkan_LIBRARY}
  glfw
  glslang
  SPIRV
)