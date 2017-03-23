//
// Copyright (c) Mario Garcia, MIT License.
//
#include "shader.hpp"
#include <glslang/Public/ShaderLang.h>
#include <SPIRV/GlslangToSpv.h>
#include <cassert>
#include <stdint.h>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>


namespace pbr {


bool ShaderModule::GLSlangInitialized = false;
const char *std_entryPoint = "main";
const char *ShaderModule::GetStdEntryPoint()
{
  return std_entryPoint;
}


void ShaderModule::CheckGLSlangInitialization()
{
  if (!ShaderModule::GLSlangInitialized) {
    glslang::InitializeProcess();
    ShaderModule::GLSlangInitialized = true;
  }
}


const TBuiltInResource DefaultTBuiltInResource = {
    /* .MaxLights = */ 32,
    /* .MaxClipPlanes = */ 6,
    /* .MaxTextureUnits = */ 32,
    /* .MaxTextureCoords = */ 32,
    /* .MaxVertexAttribs = */ 64,
    /* .MaxVertexUniformComponents = */ 4096,
    /* .MaxVaryingFloats = */ 64,
    /* .MaxVertexTextureImageUnits = */ 32,
    /* .MaxCombinedTextureImageUnits = */ 80,
    /* .MaxTextureImageUnits = */ 32,
    /* .MaxFragmentUniformComponents = */ 4096,
    /* .MaxDrawBuffers = */ 32,
    /* .MaxVertexUniformVectors = */ 128,
    /* .MaxVaryingVectors = */ 8,
    /* .MaxFragmentUniformVectors = */ 16,
    /* .MaxVertexOutputVectors = */ 16,
    /* .MaxFragmentInputVectors = */ 15,
    /* .MinProgramTexelOffset = */ -8,
    /* .MaxProgramTexelOffset = */ 7,
    /* .MaxClipDistances = */ 8,
    /* .MaxComputeWorkGroupCountX = */ 65535,
    /* .MaxComputeWorkGroupCountY = */ 65535,
    /* .MaxComputeWorkGroupCountZ = */ 65535,
    /* .MaxComputeWorkGroupSizeX = */ 1024,
    /* .MaxComputeWorkGroupSizeY = */ 1024,
    /* .MaxComputeWorkGroupSizeZ = */ 64,
    /* .MaxComputeUniformComponents = */ 1024,
    /* .MaxComputeTextureImageUnits = */ 16,
    /* .MaxComputeImageUniforms = */ 8,
    /* .MaxComputeAtomicCounters = */ 8,
    /* .MaxComputeAtomicCounterBuffers = */ 1,
    /* .MaxVaryingComponents = */ 60,
    /* .MaxVertexOutputComponents = */ 64,
    /* .MaxGeometryInputComponents = */ 64,
    /* .MaxGeometryOutputComponents = */ 128,
    /* .MaxFragmentInputComponents = */ 128,
    /* .MaxImageUnits = */ 8,
    /* .MaxCombinedImageUnitsAndFragmentOutputs = */ 8,
    /* .MaxCombinedShaderOutputResources = */ 8,
    /* .MaxImageSamples = */ 0,
    /* .MaxVertexImageUniforms = */ 0,
    /* .MaxTessControlImageUniforms = */ 0,
    /* .MaxTessEvaluationImageUniforms = */ 0,
    /* .MaxGeometryImageUniforms = */ 0,
    /* .MaxFragmentImageUniforms = */ 8,
    /* .MaxCombinedImageUniforms = */ 8,
    /* .MaxGeometryTextureImageUnits = */ 16,
    /* .MaxGeometryOutputVertices = */ 256,
    /* .MaxGeometryTotalOutputComponents = */ 1024,
    /* .MaxGeometryUniformComponents = */ 1024,
    /* .MaxGeometryVaryingComponents = */ 64,
    /* .MaxTessControlInputComponents = */ 128,
    /* .MaxTessControlOutputComponents = */ 128,
    /* .MaxTessControlTextureImageUnits = */ 16,
    /* .MaxTessControlUniformComponents = */ 1024,
    /* .MaxTessControlTotalOutputComponents = */ 4096,
    /* .MaxTessEvaluationInputComponents = */ 128,
    /* .MaxTessEvaluationOutputComponents = */ 128,
    /* .MaxTessEvaluationTextureImageUnits = */ 16,
    /* .MaxTessEvaluationUniformComponents = */ 1024,
    /* .MaxTessPatchComponents = */ 120,
    /* .MaxPatchVertices = */ 32,
    /* .MaxTessGenLevel = */ 64,
    /* .MaxViewports = */ 16,
    /* .MaxVertexAtomicCounters = */ 0,
    /* .MaxTessControlAtomicCounters = */ 0,
    /* .MaxTessEvaluationAtomicCounters = */ 0,
    /* .MaxGeometryAtomicCounters = */ 0,
    /* .MaxFragmentAtomicCounters = */ 8,
    /* .MaxCombinedAtomicCounters = */ 8,
    /* .MaxAtomicCounterBindings = */ 1,
    /* .MaxVertexAtomicCounterBuffers = */ 0,
    /* .MaxTessControlAtomicCounterBuffers = */ 0,
    /* .MaxTessEvaluationAtomicCounterBuffers = */ 0,
    /* .MaxGeometryAtomicCounterBuffers = */ 0,
    /* .MaxFragmentAtomicCounterBuffers = */ 1,
    /* .MaxCombinedAtomicCounterBuffers = */ 1,
    /* .MaxAtomicCounterBufferSize = */ 16384,
    /* .MaxTransformFeedbackBuffers = */ 4,
    /* .MaxTransformFeedbackInterleavedComponents = */ 64,
    /* .MaxCullDistances = */ 8,
    /* .MaxCombinedClipAndCullDistances = */ 8,
    /* .MaxSamples = */ 4,
    /* .limits = */ {
        /* .nonInductiveForLoops = */ 1,
        /* .whileLoops = */ 1,
        /* .doWhileLoops = */ 1,
        /* .generalUniformIndexing = */ 1,
        /* .generalAttributeMatrixVectorIndexing = */ 1,
        /* .generalVaryingIndexing = */ 1,
        /* .generalSamplerIndexing = */ 1,
        /* .generalVariableIndexing = */ 1,
        /* .generalConstantMatrixVectorIndexing = */ 1,
}};


std::string GetSource(const char *filepath)
{
  std::ifstream file;
  std::string sourceCode;
  std::string codeLine;
  file.open(filepath);
  if (file.is_open()) {
    while (std::getline(file, codeLine)) {
      sourceCode += codeLine + "\n";
    }
    file.close();
  }
  return sourceCode;
}


VkShaderModule ShaderModule::GenerateShaderModule(VkDevice device, ShaderStage stage, const char *filepath)
{
  ShaderModule::CheckGLSlangInitialization();
  VkShaderModule shaderModule;
  std::string sourceCode = GetSource(filepath);
  glslang::TProgram *program = new glslang::TProgram();
  TBuiltInResource resources = DefaultTBuiltInResource;
  EShLanguage glslLanguage;
  switch (stage) {
    case ssVertShader: glslLanguage = EShLangVertex; break;
    case ssFragShader: glslLanguage = EShLangFragment; break;
    case ssTesseShader: glslLanguage = EShLangTessEvaluation; break;
    case ssTesscShader: glslLanguage = EShLangTessControl; break;
    case ssGeomShader: glslLanguage = EShLangGeometry; break;
    case ssCompShader: glslLanguage = EShLangCompute; break;
    default: glslLanguage = EShLangVertex; break;
  }
  glslang::TShader *shader = new glslang::TShader(glslLanguage);
  const char *source = sourceCode.c_str();
  shader->setStrings(&source, 1); 
  shader->setEntryPoint(std_entryPoint);
  bool success = shader->parse(&resources, 430, false, (EShMessages )(EShMsgVulkanRules | EShMsgSpvRules));
  if (!success) {
    std::printf("%s", shader->getInfoLog());
    assert(success && "Failed to parse shader code!");
  }
  
  program->addShader(shader);
  success = program->link((EShMessages )(EShMsgVulkanRules | EShMsgSpvRules));
  assert(success && "Failed to link shader code!");
  if (!success) {
    std::printf("%s", program->getInfoLog());
  }

  std::vector<uint32_t> spirv;
  glslang::GlslangToSpv(*program->getIntermediate(glslLanguage), spirv);
  VkShaderModuleCreateInfo shaderModuleCreateInfo = { };
  shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  shaderModuleCreateInfo.pCode = spirv.data();
  shaderModuleCreateInfo.codeSize = spirv.size() * sizeof(uint32_t);
  shaderModuleCreateInfo.flags = 0;
  shaderModuleCreateInfo.pNext = nullptr;
  VkResult result = vkCreateShaderModule(device, &shaderModuleCreateInfo, nullptr, &shaderModule);
  assert(result == VK_SUCCESS && "Shader module unsuccessfully created!");

  delete program;
  delete shader;
  return shaderModule;
}
} // pbr