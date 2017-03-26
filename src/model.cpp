//
// Copyright (c) Mario Garcia, MIT License.
//
#include "model.hpp"

#include <assert.h>
// No tangent coordinates :c, but it does the job nonetheless.
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <set>
#include <iostream>

namespace pbr {


GeometryData Model::LoadModel(const char *name, const char *filepath)
{
  std::cout << "Loading up " << filepath << ".\nThis might take awhile...\n";
  GeometryData model;
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string err;

  if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filepath)) {
    std::cout << err;
    assert(false && "Failed to load model.");
  }  
  
  for (const auto &shape : shapes) {
    for (const auto &index : shape.mesh.indices) {
      Vertex vertex;
      if (index.vertex_index > -1) {
        vertex.position = {
          attrib.vertices[3 * index.vertex_index + 0],
          attrib.vertices[3 * index.vertex_index + 1],
          attrib.vertices[3 * index.vertex_index + 2]
        };
      }
      if (index.normal_index > -1) {
        vertex.normal = {
          attrib.normals[3 * index.normal_index + 0],
          attrib.normals[3 * index.normal_index + 1],
          attrib.normals[3 * index.normal_index + 2] 
        };
      }
      if (index.texcoord_index > -1) {
        vertex.uv = {
          attrib.texcoords[2 * index.texcoord_index + 0],
          attrib.texcoords[2 * index.texcoord_index + 1]
        };
      }

      
      model.vertices.push_back(vertex);
      model.indices.push_back((uint32_t )model.indices.size());
    }
  }

  std::cout << "Finished!\n";
  return model;
}
} // pbr