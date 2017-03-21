//
// Copyright (c) Mario Garcia, MIT License.
//
#ifndef __VERTEX_HPP
#define __VERTEX_HPP

#include <vector.hpp>


namespace pbr {


/// Vertex Description.
struct Vertex {
  Vec3 Position;
  Vec3 Normal;
  Vec3 Tangent;
  Vec2 TexCoords;
};
} // pbr
#endif // __VERTEX_HPP