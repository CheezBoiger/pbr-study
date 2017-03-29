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

layout (binding = 1) uniform samplerCube envMap;
layout (binding = 2) uniform samplerCube irradianceMap;

layout (binding = 4) uniform Material {
  float roughness;
  float metallic;
  float specular;
  float r;
  float g;
  float b;
} material;

struct PointLight {
  vec4 position;
  vec3 color;
  float radius;
  bool enable;
};

// Offset values are rather strange here...
layout (binding = 5) uniform Lighting {
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
// >param light information about the light.   
// >param V view direction (not normalized).
// >param L light direction (not normalized).
// >param N surface normal (not normalized).
// >param metallic the metallic value of the surface
// >param roughness the roughness value of the surface 
vec3 BRDF(vec3 V, vec3 N, vec3 L, float metallic, float roughness)
{
  vec3 nV = normalize(V);
  vec3 nL = normalize(L);
  vec3 nN = normalize(N);
  // Calculate Half vector between View and Light direction.
  vec3 H = normalize(nV + nL);

  float dotNL = clamp(dot(nN, nL), 0.0, 1.0);
  float dotNV = clamp(dot(nN, nV), 0.0, 1.0);
  float dotLH = clamp(dot(nL, H), 0.0, 1.0);
  float dotNH = clamp(dot(nN, H), 0.0, 1.0);
  
  vec3 lightColor = vec3(1.0);
  // final color output.
  vec3 color = vec3(0.0);
  vec3 F0 = vec3(0.04);
  F0 = mix(F0, vec3(material.r, material.g, material.b), metallic);
  
  // point light attenuation. handles only one light for now.
  float distance = length(L);
  float attenuation = lighting.light.radius / ((distance * distance) + 1.0);
  vec3 radiance = lighting.light.color * attenuation;
  
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
    color += (kD * vec3(material.r, material.g, material.b) / PI + brdf) * radiance * dotNL;
  }
  
  return color;
}

// Hammersley sampling algorithm by Holger Dammertz (2012)
float RadicalInverse_VdC(uint bits)
{
  bits = (bits << 16u) | (bits >> 16u);
  bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
  bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
  bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
  bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
  return float(bits) * 2.3283064365386963e-10; // 0x100000000
}

vec2 Hammersley(uint i, uint N)
{
  return vec2(float(i)/float(N), RadicalInverse_VdC(i));
}


// IBL Algorithm used by Unreal.
//
vec3 ImportanceSampleGGX(vec2 xI, float roughness, vec3 N)
{
  float alpha = roughness * roughness;
  float phi = 2 * PI * xI.x;
  float cosTheta = sqrt((1 - xI.y) / (1 + (alpha*alpha - 1) * xI.y));
  float sinTheta = sqrt(1 - cosTheta * cosTheta);
  
  vec3 H;
  H.x = sinTheta * cos(phi);
  H.y = sinTheta * sin(phi);
  H.z = cosTheta;
  
  vec3 upVector = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
  vec3 tangentX = normalize(cross(upVector, N));
  vec3 tangentY = cross(N, tangentX);
  
  return tangentX * H.x + tangentY * H.y + N * H.z;
}

vec3 PrefilterEnvMap(float roughness, vec3 R)
{
  vec3 N = R;
  vec3 V = R;
  
  vec3 prefilteredColor = vec3(0.0);
  float totalWeight = 0.0;
  
  const uint kNumSamples = 128;
  for (uint i = 0; i < kNumSamples; ++i) {
    vec2 xI = Hammersley(i, kNumSamples);
    vec3 H = ImportanceSampleGGX(xI, roughness, N);
    vec3 L = 2 * dot(V, H) * H - V;
    
    float NoL = clamp(dot(N, L), 0.0, 1.0);
    if (NoL > 0) {
      prefilteredColor += texture(envMap, L).rgb * NoL;
      totalWeight += NoL;
    }
  }
  
  return prefilteredColor / totalWeight;
}

// Non integrate.
vec2 IntegrateBRDF(float roughness, float NoV, vec3 N)
{
  vec3 V;
  V.x = sqrt(1.0 - NoV * NoV);
  V.y = 0.0;
  V.z = NoV;
  
  float A = 0.0;
  float B = 0.0;
  
  const uint kNumSamples = 10;
  for (uint i = 0; i < kNumSamples; ++i) {
    vec2 xI = Hammersley(i, kNumSamples);
    vec3 H = ImportanceSampleGGX(xI, roughness, N);
    vec3 L = 2 * dot(V, H) * H - V;
    
    float NoL = clamp(L.z, 0.0, 1.0);
    float NoH = clamp(H.z, 0.0, 1.0);
    float VoH = clamp(dot(V, H), 0.0, 1.0);
    
    if (NoL > 0) {
      float G = GSchlickmithGGX(NoL, NoV, roughness);
      float G_Vis = G * VoH / (NoH * NoV);
      float Fc = pow(1 - VoH, 5);
      A += (1 - Fc) * G_Vis;
      B += Fc * G_Vis;
    }
  }
  
  return vec2(A, B) / kNumSamples;
}

// Environment BRDF approximation from 
// https://www.unrealengine.com/blog/physically-based-shading-on-mobile
vec3 EnvBRDFApprox(vec3 SpecularColor, float Roughness, float NoV) 
{
	vec4 c0 = vec4(-1, -0.0275, -0.572, 0.022);
	vec4 c1 = vec4(1, 0.0425, 1.04, -0.04);
	vec4 r = Roughness * c0 + c1;
	float a004 = min(r.x * r.x, exp2(-9.28 * NoV)) * r.x + r.y;
	vec2 AB = vec2(-1.04, 1.04) * a004 + r.zw;
	return SpecularColor * AB.x + AB.y;
}

vec3 ApproximateSpecularIBL(vec3 specularColor, float roughness, vec3 N, vec3 V)
{
  vec3 nN = normalize(N);
  vec3 nV = normalize(V);
  float NoV = clamp(dot(nN, nV), 0.0, 1.0);
  vec3 R = 2 * dot(nV, nN) * nN - nV;
  
  vec3 prefilteredColor = PrefilterEnvMap(roughness, R);
  vec2 EnvBRDF = IntegrateBRDF(roughness, NoV, nN);
  return  prefilteredColor * (specularColor );//* EnvBRDF.x + EnvBRDF.y);
}

void main() 
{
  vec3 V = normalize(ubo.camPosition - fragPos);
  vec3 L = lighting.light.position.xyz - fragPos;
  vec3 N = normalize(fragNormal);
  vec3 R = reflect(-V, N);

  ivec2 cubedim = textureSize(envMap, 0);
  int numMipLevels = int(log2(max(cubedim.s, cubedim.y)));
  float mipLevel = numMipLevels - 1.0 + log2(material.roughness);
  
  vec3 baseColor = vec3(material.r, material.g, material.b);
  vec3 diffuseColor = baseColor - baseColor * material.metallic;
  vec3 specularColor = mix(vec3(material.specular), baseColor, material.metallic);
  
  vec3 radianceSample = pow(textureLod(envMap, R, mipLevel).rgb, vec3(2.2));
  vec3 irradianceSample = pow(texture(irradianceMap, N).rgb, vec3(2.2));
  
  //vec3 color = ApproximateSpecularIBL(specularColor, material.roughness, N, V);
 
  vec3 reflection = EnvBRDFApprox(specularColor, material.roughness, clamp(dot(N, V), 0.0, 1.0));
  
  vec3 diffuse = diffuseColor * irradianceSample;
  vec3 specular = radianceSample * reflection;
  vec3 color = diffuse + specular;
  if (lighting.light.enable) {
    color += BRDF(V, N, L, material.metallic, material.roughness);
  }
   // calculate for gamma correction and hdr rendering.
  color = color / (color + vec3(1.0));
  color = pow(color, vec3(1.0/2.2));
  outColor = vec4(color, 1.0f);//texture(image, fragTexCoord);
}






