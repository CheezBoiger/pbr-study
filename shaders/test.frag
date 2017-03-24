//
// Copyright (c) Mario Garcia, MIT License.
//
// This Shader handles PBR (Physically Based Rendering) shading and
// uses the same PBR shading model as Disney's animated films and the Unreal Engine.

// PBR is a popular trend in Computer Graphics as it models real world
// lighting, and shading. Look at the followng Lecture notes by Epic Games,
// creators of the Unreal Engine, for more information about the theory:
//
//  http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
//
#version 430 
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 fragPos;
layout (location = 1) in vec3 fragNormal;
layout (location = 2) in vec2 fragTexCoord;

layout (location = 0) out vec4 outColor;

layout (binding = 0) uniform UniformBufferObject {
  mat4 model;
  mat4 view;
  mat4 projection;
  vec3 camPosition;
} ubo;

layout (binding = 1) uniform sampler2D image;

layout (binding = 2) uniform Material {
  float roughness;
  float metallic;
  float r;
  float g;
  float b;
} material;

struct PointLight {
  vec4 position;
  vec3 color;
  float radius;
};

// Offset values are rather strange here...
layout (binding = 3) uniform Lighting {
  PointLight light;
} lighting;

const float PI = 3.14159265359;


// GGX from Trowbridge-Reitz
float DGGX(float NoH, float roughness)
{
  float alpha = (roughness * roughness);
  float alpha2 = (alpha * alpha);
  float denom = (NoH * NoH) * (alpha2 - 1.0) + 1.0;
  return alpha2 / (PI * (denom * denom));
}


// Geometric Shadowing with Schlick-SmithGGX 
float GSchlickmithGGX(float NoL, float NoV, float roughness)
{
  // We are using direct lighting in this case, so Disney's implementation of k remap will suffice.
  // If doing Image based lighting, IBL, we would go with roughness^2 / 2.
  float remap = roughness + 1.0;
  float k = (remap * remap) / 8.0;
  float GL = NoL / (NoL * (1.0 - k) + k);
  float GV = NoV / (NoV * (1.0 - k) + k);
  return GL * GV;
}


// Schlick Approximation for the Fresnel Term.
vec3 FSchlick(float cosTheta, vec3 F0)
{
  return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}


// BRDF Cook-Torrance model. you can find this reference in the following website:
//
// http://graphicrants.blogspot.com/2013/08/specular-brdf-reference.html
// 
// If you are interested in the work, It is a model adopted by Epic Games for the 
// Unreal Engine shading model, you can read up on the theory here:
//
// http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
//
// In case you were wondering, a BRDF is acronym for Bidirectional Reflectance Distribution
// Function. It describes the way light behaves when reflecting off a surface. Light isn't 
// always reflecting off perfectly smooth surfaces, so a model was created to represent 
// a sort of rough surface (as all objects have a sort of non-smooth surface), with little 
// mirrors we can call "microfacets", or micro surfaces if you prefer.   
// >param V view direction.
// >param L light direction.
// >param N surface normal.
// >param metallic the metallic value of the surface
// >param roughness the roughness value of the surface 
vec3 BRDF(vec3 V, vec3 N, vec3 L, float metallic, float roughness)
{
  // Calculate Half vector between View and Light direction.
  vec3 H = normalize(V + L);

  float dotNL = clamp(dot(N, L), 0.0, 1.0);
  float dotNV = clamp(dot(N, V), 0.0, 1.0);
  float dotLH = clamp(dot(L, H), 0.0, 1.0);
  float dotNH = clamp(dot(N, H), 0.0, 1.0);
  
  vec3 lightColor = vec3(1.0);
  // final color output.
  vec3 color = vec3(0.0);
  vec3 F0 = vec3(0.04);
  F0 = mix(F0, vec3(material.r, material.g, material.b), metallic);
  float distance = length(L);
  float attenuation = lighting.light.radius / ((distance * distance) + 1.0);
  vec3 radiance = vec3(1.0) * attenuation;
  
  if (dotNL > 0.0) {
    float D = DGGX(dotNH, roughness);
    float G = GSchlickmithGGX(dotNL, dotNV, roughness);
    vec3 F = FSchlick(dotNV, F0);
    // Cook Torrance microfacet specular BRDF
    vec3 brdf = D * F * G / (4 * dotNL * dotNV);
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;
    //color += brdf * dotNL * lighting.light.color;
    color += ((kD * vec3(material.r, material.g, material.b)) / PI + brdf) * radiance * dotNL;
  }
  
  return color;
}

void main() {
  vec3 V = normalize(ubo.camPosition - fragPos);
  vec3 L = normalize(lighting.light.position.xyz - fragPos);
  vec3 N = normalize(fragNormal);
  vec3 color = BRDF(V, N, L, material.metallic, material.roughness);
  color += vec3(material.r, material.g, material.b) * 0.02;
  
   // calculate for gamma correction and hdr rendering.
  color = color / (color + vec3(1.0));
  color = pow(color, vec3(1.0/2.2));
  outColor = vec4(color, 1.0f);//texture(image, fragTexCoord);
}