//
// Copyright (c) Mario Garcia, MIT License.
// 
#include "geometry.hpp"
#include <cmath>
#include <algorithm>


// Constants from math.h
#define PI       3.14159265358979323846   // pi
#define PI_2     1.57079632679489661923   // pi/2
#define PI_4 0.785398163397448309616 // pi/4
#define OCTAHEDRON 0

namespace pbr {


GeometryData Geometry::CreateSphere(float radius, uint32_t sliceCount, uint32_t stackCount)
{
  //subdivisions = (std::min)(subdivisions, 5u);
  GeometryData meshData;
  meshData.vertices.clear();
  meshData.indices.clear();

  //
  // Compute the vertices stating at the top pole and moving down the stacks.
  //

  // Poles: note that there will be texture coordinate distortion as there is
  // not a unique point on the texture map to assign to the pole when mapping
  // a rectangular texture onto a sphere.
  Vertex topVertex;
  topVertex.position = glm::vec3(0.0f, +radius, 0.0f);
  topVertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);
  topVertex.uv = glm::vec2(0.0f, 0.0f);

  Vertex bottomVertex;
  bottomVertex.position = glm::vec3(0.0f, -radius, 0.0f);
  bottomVertex.normal = glm::vec3(0.0f, -1.0f, 0.0f);
  bottomVertex.uv = glm::vec2(0.0f, 1.0f);

  meshData.vertices.push_back(topVertex);

  float phiStep = float(PI) / stackCount;
  float thetaStep =  float(2.0f * PI) / sliceCount;

  // Compute vertices for each stack ring (do not count the poles as rings).
  for (uint32_t i = 1; i <= stackCount - 1; ++i)
  {
    float phi = i*phiStep;

    // Vertices of ring.
    for (uint32_t j = 0; j <= sliceCount; ++j)
    {
      float theta = j*thetaStep;

      Vertex v;

      // spherical to cartesian
      v.position.x = radius*sinf(phi)*cosf(theta);
      v.position.y = radius*cosf(phi);
      v.position.z = radius*sinf(phi)*sinf(theta);

      glm::vec3 p = glm::vec3(v.position);
      v.normal = glm::normalize(p);

      v.uv.x = theta / float(PI_2);
      v.uv.y = phi / float(PI);

      meshData.vertices.push_back(v);
    }
  }

  meshData.vertices.push_back(bottomVertex);

  //
  // Compute indices for top stack.  The top stack was written first to the vertex buffer
  // and connects the top pole to the first ring.
  //

  for (uint32_t i = 1; i <= sliceCount; ++i)
  {
    meshData.indices.push_back(0);
    meshData.indices.push_back(i + 1);
    meshData.indices.push_back(i);
  }

  //
  // Compute indices for inner stacks (not connected to poles).
  //

  // Offset the indices to the index of the first vertex in the first ring.
  // This is just skipping the top pole vertex.
  uint32_t baseIndex = 1;
  uint32_t ringVertexCount = sliceCount + 1;
  for (uint32_t i = 0; i < stackCount - 2; ++i)
  {
    for (uint32_t j = 0; j < sliceCount; ++j)
    {
      meshData.indices.push_back(baseIndex + i*ringVertexCount + j);
      meshData.indices.push_back(baseIndex + i*ringVertexCount + j + 1);
      meshData.indices.push_back(baseIndex + (i + 1)*ringVertexCount + j);

      meshData.indices.push_back(baseIndex + (i + 1)*ringVertexCount + j);
      meshData.indices.push_back(baseIndex + i*ringVertexCount + j + 1);
      meshData.indices.push_back(baseIndex + (i + 1)*ringVertexCount + j + 1);
    }
  }

  //
  // Compute indices for bottom stack.  The bottom stack was written last to the vertex buffer
  // and connects the bottom pole to the bottom ring.
  //

  // South pole vertex was added last.
  uint32_t southPoleIndex = (uint32_t)meshData.vertices.size() - 1;

  // Offset the indices to the index of the first vertex in the last ring.
  baseIndex = southPoleIndex - ringVertexCount;

  for (uint32_t i = 0; i < sliceCount; ++i)
  {
    meshData.indices.push_back(southPoleIndex);
    meshData.indices.push_back(baseIndex + i);
    meshData.indices.push_back(baseIndex + i + 1);
  }
  return meshData;
}


GeometryData Geometry::CreateCube()
{
  GeometryData data;

  std::vector<glm::vec3> positions = std::initializer_list<glm::vec3>{
    // Front
    glm::vec3(-1.0f, -1.0f, 1.0f),
    glm::vec3(1.0f, -1.0f, 1.0f),
    glm::vec3(1.0f, 1.0f, 1.0f),
    glm::vec3(1.0f, 1.0f, 1.0f),
    glm::vec3(-1.0f, 1.0f, 1.0f),
    glm::vec3(-1.0f, -1.0f, 1.0f),
    // Back
    glm::vec3(-1.0f, -1.0f, -1.0f),
    glm::vec3(-1.0f, 1.0f, -1.0f),
    glm::vec3(1.0f, 1.0f, -1.0f),
    glm::vec3(1.0f, 1.0f, -1.0f),
    glm::vec3(1.0f, -1.0f, -1.0f),
    glm::vec3(-1.0f, -1.0f, -1.0f),
    // Up
    glm::vec3(1.0f, 1.0f, 1.0f),
    glm::vec3(1.0f, 1.0f, -1.0f),
    glm::vec3(-1.0f, 1.0f, -1.0f),
    glm::vec3(-1.0f, 1.0f, -1.0f),
    glm::vec3(-1.0f, 1.0f, 1.0f),
    glm::vec3(1.0f, 1.0f, 1.0f),
    // Down
    glm::vec3(1.0f, -1.0f, 1.0f),
    glm::vec3(-1.0f, -1.0f, 1.0f),
    glm::vec3(-1.0f, -1.0f, -1.0f),
    glm::vec3(-1.0f, -1.0f, -1.0f),
    glm::vec3(1.0f, -1.0f, -1.0f),
    glm::vec3(1.0f, -1.0f, 1.0f),
    // Right
    glm::vec3(1.0f, -1.0f, 1.0f),
    glm::vec3(1.0f, -1.0f, -1.0f),
    glm::vec3(1.0f, 1.0f, -1.0f),
    glm::vec3(1.0f, 1.0f, -1.0f),
    glm::vec3(1.0f, 1.0f, 1.0f),
    glm::vec3(1.0f, -1.0f, 1.0f),
    // Left
    glm::vec3(-1.0f, -1.0f, 1.0f),
    glm::vec3(-1.0f, 1.0f, 1.0f),
    glm::vec3(-1.0f, 1.0f, -1.0f),
    glm::vec3(-1.0f, 1.0f, -1.0f),
    glm::vec3(-1.0f, -1.0f, -1.0f),
    glm::vec3(-1.0f, -1.0f, 1.0f),
  };

  // Still need to create normals.
  std::vector<glm::vec3> normals = std::initializer_list<glm::vec3>{
    // Front 
    glm::vec3(0.0f, 0.0f, 1.0f),
    glm::vec3(0.0f, 0.0f, 1.0f),
    glm::vec3(0.0f, 0.0f, 1.0f),
    glm::vec3(0.0f, 0.0f, 1.0f),
    glm::vec3(0.0f, 0.0f, 1.0f),
    glm::vec3(0.0f, 0.0f, 1.0f),
    // Back
    glm::vec3(0.0f, 0.0f, -1.0f),
    glm::vec3(0.0f, 0.0f, -1.0f),
    glm::vec3(0.0f, 0.0f, -1.0f),
    glm::vec3(0.0f, 0.0f, -1.0f),
    glm::vec3(0.0f, 0.0f, -1.0f),
    glm::vec3(0.0f, 0.0f, -1.0f),
    // Up
    glm::vec3(0.0f, 1.0f, 0.0f),
    glm::vec3(0.0f, 1.0f, 0.0f),
    glm::vec3(0.0f, 1.0f, 0.0f),
    glm::vec3(0.0f, 1.0f, 0.0f),
    glm::vec3(0.0f, 1.0f, 0.0f),
    glm::vec3(0.0f, 1.0f, 0.0f),
    // Down
    glm::vec3(0.0f, -1.0f, 0.0f),
    glm::vec3(0.0f, -1.0f, 0.0f),
    glm::vec3(0.0f, -1.0f, 0.0f),
    glm::vec3(0.0f, -1.0f, 0.0f),
    glm::vec3(0.0f, -1.0f, 0.0f),
    glm::vec3(0.0f, -1.0f, 0.0f),
    // Right
    glm::vec3(1.0f, 0.0f, 0.0f),
    glm::vec3(1.0f, 0.0f, 0.0f),
    glm::vec3(1.0f, 0.0f, 0.0f),
    glm::vec3(1.0f, 0.0f, 0.0f),
    glm::vec3(1.0f, 0.0f, 0.0f),
    glm::vec3(1.0f, 0.0f, 0.0f),
    // Left
    glm::vec3(-1.0f, 0.0f, 0.0f),
    glm::vec3(-1.0f, 0.0f, 0.0f),
    glm::vec3(-1.0f, 0.0f, 0.0f),
    glm::vec3(-1.0f, 0.0f, 0.0f),
    glm::vec3(-1.0f, 0.0f, 0.0f),
    glm::vec3(-1.0f, 0.0f, 0.0f)
  };

  std::vector<glm::vec2> uvs = std::initializer_list<glm::vec2>{
    glm::vec2(0.0f, 0.0f),
    glm::vec2(1.0f, 0.0f),
    glm::vec2(1.0f, 1.0f),
    glm::vec2(1.0f, 1.0f),
    glm::vec2(0.0f, 1.0f),
    glm::vec2(0.0f, 0.0f),

    glm::vec2(0.0f, 0.0f),
    glm::vec2(1.0f, 0.0f),
    glm::vec2(1.0f, 1.0f),
    glm::vec2(1.0f, 1.0f),
    glm::vec2(0.0f, 1.0f),
    glm::vec2(0.0f, 0.0f),

    glm::vec2(0.0f, 0.0f),
    glm::vec2(1.0f, 0.0f),
    glm::vec2(1.0f, 1.0f),
    glm::vec2(1.0f, 1.0f),
    glm::vec2(0.0f, 1.0f),
    glm::vec2(0.0f, 0.0f),

    glm::vec2(0.0f, 0.0f),
    glm::vec2(1.0f, 0.0f),
    glm::vec2(1.0f, 1.0f),
    glm::vec2(1.0f, 1.0f),
    glm::vec2(0.0f, 1.0f),
    glm::vec2(0.0f, 0.0f),

    glm::vec2(0.0f, 0.0f),
    glm::vec2(1.0f, 0.0f),
    glm::vec2(1.0f, 1.0f),
    glm::vec2(1.0f, 1.0f),
    glm::vec2(0.0f, 1.0f),
    glm::vec2(0.0f, 0.0f),

    glm::vec2(0.0f, 0.0f),
    glm::vec2(1.0f, 0.0f),
    glm::vec2(1.0f, 1.0f),
    glm::vec2(1.0f, 1.0f),
    glm::vec2(0.0f, 1.0f),
    glm::vec2(0.0f, 0.0f),
  };

  std::vector<uint32_t> indices = std::initializer_list<uint32_t>{
    0, 1, 2,
    3, 4, 5,
    6, 7, 8,
    9, 10, 11,
    12, 13, 14,
    15, 16, 17,
    18, 19, 20,
    21, 22, 23,
    24, 25, 26,
    27, 28, 29,
    30, 31, 32,
    33, 34, 35
  };

  data.vertices.resize(positions.size());
  for (uint32_t i = 0; i < positions.size(); ++i) {
    data.vertices[i].position = positions[i];
    data.vertices[i].normal = normals[i];
    data.vertices[i].uv = uvs[i];
  }
  data.indices.resize(indices.size());
  for (uint32_t i = 0; i < indices.size(); ++i) {
    data.indices[i] = indices[i];
  }
  return data;
}
} // pbr 