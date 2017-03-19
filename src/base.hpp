//
// Copyright (c) Mario Garcia, MIT License.
//
#ifndef __BASE_HPP
#define __BASE_HPP


#include <stdint.h>
#include "platform.hpp"
#include <vulkan/vulkan.h>


namespace pbr {
namespace global {

VkInstance instance;

}


/// Base class handles simple base stuff...
class Base {
public:
  Base();
  virtual ~Base() { }

  void Initialize();
  virtual void Render();
  virtual void Cleanup();


  void Display();

protected:

  void CreateSurface();
  void CreateLogicalDevice();
  void FindPhyiscalDevice();

  struct {
    VkQueue presentation;
    VkQueue rendering;
  } m_queue;
  
  VkPhysicalDevice  m_phyDev;
  VkDevice          m_logDev;
};
} // pbr
#endif // __BASE_HPP