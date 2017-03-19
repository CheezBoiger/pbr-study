//
// Copyright (c) Mario Garcia, MIT License.
//
#ifndef __BASE_HPP
#define __BASE_HPP


#include <stdint.h>
#include "platform.hpp"
#include "camera.hpp"
#include <vulkan/vulkan.h>


struct GLFWwindow;


namespace pbr {
namespace global {


extern VkInstance GetInstance();
typedef struct GLFWwindow *Window;
} // global


/// Base class handles simple base stuff...
class Base {
public:
  Base();
  virtual ~Base() { }

  void Initialize();
  virtual void Run();
  virtual void Cleanup();


  void SetupWindow(uint32_t width, uint32_t height);
  void CloseWindow();
  
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
  global::Window    m_window;
  Camera            m_camera;
  double            m_lastTime;
  double            m_dt;
};
} // pbr
#endif // __BASE_HPP