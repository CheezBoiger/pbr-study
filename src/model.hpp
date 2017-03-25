//
// Copyright (c) Mario Garcia, MIT License.
//
#ifndef __MODEL_HPP
#define __MODEL_HPP


#include "platform.hpp"
#include "vertex.hpp"
#include "geometry.hpp"
#include <vector>
#include <string>

namespace pbr {


class Mesh {
public:

private:
};


class Texture {
};


class Model {
public:
  static GeometryData LoadModel(const char *name, const char *filepath);

  GeometryData &GetData() { return data; }
  
  // oh boy wish i could use string view...
  std::string GetName() { return name; }
  
private:
  std::string name;
  GeometryData data;  
};
} // pbr
#endif // __MODEL_HPP