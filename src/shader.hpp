//
// Copyright (c) Mario Garcia, MIT License.
//
#ifndef __SHADER_MODULE_HPP
#define __SHADER_MODULE_HPP


#include "platform.hpp"
#include <vulkan/vulkan.h>


namespace pbr {


/// Shader Module generates a module for your graphics pipeline.
class ShaderModule {
public:
  /// Specify the Shaderstage.
  enum ShaderStage {
    ssVertShader,
    ssFragShader,
    ssTesseShader,
    ssTesscShader,
    ssGeomShader,
    ssCompShader,
  };

  /// Generates a Shader Module with the filepath and stage definition.
  /// @param stage
  /// @param filepath Filepath to the shader code, must be glsl!
  static VkShaderModule GenerateShaderModule(VkDevice device, ShaderStage stage, const char *filepath);
  static const char *GetStdEntryPoint();

private:
  static void CheckGLSlangInitialization();
  static bool GLSlangInitialized;
};
} // pbr
#endif // __SHADER_MODULE_HPP