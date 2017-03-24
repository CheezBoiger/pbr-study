#version 430
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texcoord;

layout (location = 0) out vec3 fragPos;
layout (location = 1) out vec3 fragNormal;
layout (location = 2) out vec2 fragTexCoord;


layout (binding = 0) uniform UniformBufferObject {
  mat4 model;
  mat4 view;
  mat4 projection;
  vec3 camPosition;
} ubo;


void main() {
  vec4 worldPosition = ubo.model * vec4(position, 1.0);
  gl_Position = ubo.projection * ubo.view * worldPosition;
  fragPos = worldPosition.xyz;
  fragNormal = mat3(transpose(inverse(ubo.model))) * normal;
  fragTexCoord = texcoord;
}