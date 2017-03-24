#version 430
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec2 texcoord;

layout (location = 0) out vec3 fragColor;
layout (location = 1) out vec2 fragTexCoord;


layout (binding = 0) uniform UniformBufferObject {
  mat4 model;
  mat4 view;
  mat4 projection;
} ubo;


void main() {
  gl_Position = ubo.projection * ubo.view * ubo.model * vec4(position, 1.0);
  fragColor = color;
  fragTexCoord = texcoord;
}