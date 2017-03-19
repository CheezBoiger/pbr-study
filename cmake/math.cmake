# Copyright (c) Mario Garcia, MIT License.

include_directories(SYSTEM 
  ${CMAKE_SOURCE_DIR}/math/impl
)


set(MATH_DIR 
  ${CMAKE_SOURCE_DIR}/math/matrix.hpp
  ${CMAKE_SOURCE_DIR}/math/quaternion.hpp
  ${CMAKE_SOURCE_DIR}/math/ray.hpp
  ${CMAKE_SOURCE_DIR}/math/vector.hpp
  ${CMAKE_SOURCE_DIR}/math/impl/matrix.inl
  ${CMAKE_SOURCE_DIR}/math/impl/vector.inl
  ${CMAKE_SOURCE_DIR}/math/impl/quaternion.inl
  ${CMAKE_SOURCE_DIR}/math/impl/ray.inl
)