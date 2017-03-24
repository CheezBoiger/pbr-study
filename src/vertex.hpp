//
// Copyright (c) Mario Garcia, MIT License.
//
#ifndef __VERTEX_HPP
#define __VERTEX_HPP

#include <vector.hpp>
#include <glm/glm.hpp>
#include <glm/common.hpp>
#include <glm/gtc/matrix_transform.hpp>


namespace pbr {


/// Vertex Description.
struct Vertex {
  glm::vec3 Position;
  glm::vec3 Normal;
  glm::vec2 uv;
};
} // pbr
#endif // __VERTEX_HPP