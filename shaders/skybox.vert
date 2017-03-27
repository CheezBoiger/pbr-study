//
// Copyright (c) Mario Garcia, MIT License.
//
#version 430 
#extension GL_ARB_separate_shader_objects : enable
layout (location = 0) in vec3 inPosition;
layout (location = 2) in vec2 inUV;
layout (location = 0) out vec3 outPosition;

layout (binding = 0) uniform UniformBufferObject {
  mat4 model;
  mat4 view;
  mat4 projection;
  vec3 camPosition;
} ubo;

void main() 
{
  gl_Position =  ubo.projection * ubo.model * vec4(inPosition, 1.0);
  outPosition = inPosition;
}