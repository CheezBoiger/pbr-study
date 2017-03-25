//
// Copyright (c) Mario Garcia, Under the MIT License.
//
#ifndef __GEOMETRY_HPP
#define __GEOMETRY_HPP


#include "vertex.hpp"
#include <vector>


namespace pbr {


struct GeometryData {
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;
};

class Geometry {
public:

  static GeometryData CreateSphere(float radius, uint32_t sliceCount, uint32_t stackCount);
};
} // pbr
#endif // __GEOMETRY_HPP