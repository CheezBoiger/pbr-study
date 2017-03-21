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
/// This service as a simple base class for initializing the KHR surface
/// for rendering, framebuffers, renderpasses, semaphores, queues, etc for our 
/// overall rendering engine. In order to create a more proper engine, we can abstract 
/// alot of the Render API calls, but this would require time, blood, sweat and tears,
/// so I'll leave that for my Vikr Renderer API. 
class Base {
public:
  Base();
  virtual ~Base();

  /// Initialize the Base members. It is important you call this function first before
  /// proceeding ahead with Run(), otherwise nothing will work, lulz.
  virtual void Initialize();

  /// Run the Rendering loop. This also presents the frames to the display for the user.
  /// Be sure you call Initialize() first, before doing anything with this function.
  virtual void Run();

  /// Clean up the Rendering engine. This is called implicitly.
  virtual void Cleanup();

  /// Get the Camera that is set up with this Base class. No need to create your 
  /// own camera, unless you plan on doing some sick shots with multiple cameras in
  /// different angles.
  Camera *GetCamera() { return &m_camera; }

  VkBuffer CreateBuffer();
  VkQueue CreateQueue();

  void SetupWindow(uint32_t width, uint32_t height);
  void CloseWindow();
  
protected:
  /// QueueFamily indices, stores the index to the 
  /// device's queue family that represent a certain purpose, such as compute work or 
  /// graphics, memory transfer, etc. In this case, a queue family describes the 
  /// family of queues that allow for THAT sort of solo purpose. Of course, queues aren't
  /// yet created, we use the index to the queue family, that is stored in here, to create that
  /// specific queue for us. We then need to get the handle to that queue with vkGetDeviceQueue() 
  struct QueueFamilyIndices {
    int graphicsFamily = -1;
    int presentFamily = -1;
    bool IsComplete();
  };

  /// Find our supported queue families, stored in the physical device.
  /// A default -1 is stored if no queue family exists with a graphics bit.
  QueueFamilyIndices FindQueueFamilies();

  void CreateSurface();
  void CreateLogicalDevice();
  void FindPhyiscalDevice();
  void SetDebugCallback();
  uint32_t FindMemoryType(uint32_t type);

  /// Device queue.
  struct {
    VkQueue presentation;
    VkQueue rendering;
  } m_queue;

  /// Device semaphores.
  struct {
    VkSemaphore present;
    VkSemaphore rendering;
  } m_semaphores;
  
  VkPhysicalDevice  m_phyDev;
  VkDevice          m_logDev;
  VkSurfaceKHR      m_surface;
  VkCommandPool     m_commandPool;
  VkDebugReportCallbackEXT m_callback;
  VkDeviceMemory    m_commandMem;
  global::Window    m_window;
  Camera            m_camera;
  double            m_lastTime;
  double            m_dt;
};
} // pbr
#endif // __BASE_HPP