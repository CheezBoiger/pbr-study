//
// Copyright (c) Mario Garcia, MIT License.
//
#version 430
#extension GL_ARB_separate_shader_objects : enable
layout (location = 0) in vec3 inPosition;

layout (location = 0) out vec4 outColor;


layout (binding = 0) uniform UniformBufferObject {
  mat4 model;
  mat4 view;
  mat4 projection;
  vec3 camPosition;
} ubo;

layout (binding = 1) uniform samplerCube envMap;

void main()
{
  outColor = vec4(1.0);
}