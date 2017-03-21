//
// Copyright (c) Mario Garcia, MIT License.
//
#include "base.hpp"
#include <GLFW/glfw3.h>

#include <string>
#include <set>
#include <vector>
#include <assert.h>
#include <iostream>


#define BASE_DEBUG
#if defined(BASE_DEBUG)
 #define BASE_ASSERT(expr) assert(expr)
#else
 #define BASE_ASSERT(expr)
#endif  

// Vulkan Validation Layer Callback.
static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
  VkDebugReportFlagsEXT flags,
  VkDebugReportObjectTypeEXT objType,
  uint64_t obj,
  size_t location,
  int32_t code,
  const char *layerPrefix,
  const char *msg,
  void *usrData)
{
  std::printf("Validation => %s\n", msg);
  std::printf("Pressing Enter will continue with the program...\n");
  std::cin.ignore();
  return VK_FALSE;
}


// Helper to find the address of vkCreateDebugReportCallbackExt, it 
// is not defined by the VulkanAPI.
VkResult CreateDebugReportCallbackEXT(VkInstance instance, 
  const VkDebugReportCallbackCreateInfoEXT *pCreateInfo,
  const VkAllocationCallbacks *pAllocator, VkDebugReportCallbackEXT *pCallback)
{
  auto func = (PFN_vkCreateDebugReportCallbackEXT ) 
    vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
  if (func != nullptr) {
    return func(instance, pCreateInfo, pAllocator, pCallback);
  } else {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}


void DestroyDebugReportCallbackEXT(VkInstance instance,
  VkDebugReportCallbackEXT callback, const VkAllocationCallbacks *pAllocator)
{
  auto func = (PFN_vkDestroyDebugReportCallbackEXT ) 
    vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
  if (func != nullptr) {
    func(instance, callback, pAllocator);
  }
}


namespace pbr {
namespace global {


const std::vector<const char *> validationLayers { "VK_LAYER_LUNARG_standard_validation" };
#if defined(BASE_DEBUG)
const bool enableValidationLayers = true;
#else
const bool enableValidationLayers = false;
#endif


std::vector<const char *> GetRequiredExtensions()
{
  std::vector<const char *> extensions;
  uint32_t glfwExtensionsCount = 0;
  const char **glfwExtensions;
  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionsCount);
  for (uint32_t i = 0; i < glfwExtensionsCount; ++i) {
    extensions.push_back(glfwExtensions[i]);
  }

  if (enableValidationLayers) {
    extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
  }
  
  return extensions;
}


/// Check for validation support.
bool CheckValidationLayerSupport()
{
  uint32_t layerCount = 0;
  vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
  std::vector<VkLayerProperties> availableLayers(layerCount);
  vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

  for (const char *layerName : validationLayers) {
    bool layerFound = false;
    for (const auto &layerProperties : availableLayers) {
      if (std::strcmp(layerName, layerProperties.layerName) == 0) {
        layerFound = true;
        break;
      }  
    }
    if (!layerFound) {
      return false;
    }
  }
  return true;
}


VkInstance CreateInstance()
{
  glfwInit();
  if (enableValidationLayers && !CheckValidationLayerSupport()) {
    std::printf("this shit don't work mate...\n");
  }

  VkApplicationInfo appInfo = { };
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.apiVersion = VK_API_VERSION_1_0;
  appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.pApplicationName = "Physically Based Rendering Study";
  appInfo.pEngineName = "StupidEngine (TM)";

  VkInstanceCreateInfo instanceCreateInfo = { };
  instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instanceCreateInfo.pApplicationInfo = &appInfo;
  
  auto extensions = GetRequiredExtensions();
  instanceCreateInfo.enabledExtensionCount = extensions.size();
  instanceCreateInfo.ppEnabledExtensionNames = extensions.data();
  if (enableValidationLayers) {
    instanceCreateInfo.enabledLayerCount = validationLayers.size();
    instanceCreateInfo.ppEnabledLayerNames = validationLayers.data();
  } else {
    instanceCreateInfo.enabledLayerCount = 0;
  }
  VkInstance inst;
  VkResult result = vkCreateInstance(&instanceCreateInfo, nullptr, &inst);
  BASE_ASSERT(result == VK_SUCCESS);
  return inst;
}

VkInstance instance = CreateInstance();


VkInstance GetInstance()
{
  return instance;
}


void KeyCallback(Window window, int key, int scancode, int action, int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  }
  if (action == GLFW_PRESS) {
    switch (key) {
      case GLFW_KEY_C: 
      {
#if defined(_WIN32)
        HWND console = GetConsoleWindow();
        ShowWindow(console, SW_HIDE);
#endif
      }
      break; 
      case GLFW_KEY_V:  
      {
#if defined(_WIN32)
        HWND console = GetConsoleWindow();
        ShowWindow(console, SW_SHOWNORMAL);
#endif
      }
      break;
      default: break;
    }
  }
}
} // global


Base::Base()
{
  glfwInit();
}


Base::~Base()
{
  CloseWindow();
  Cleanup();/*
  vkDestroyCommandPool(m_logDev, m_commandPool, nullptr);
  vkDestroySemaphore(m_logDev, m_semaphores.present, nullptr);
  vkDestroySemaphore(m_logDev, m_semaphores.rendering, nullptr);
*/
  vkDestroySurfaceKHR(global::GetInstance(), m_surface, nullptr);
  vkDestroyDevice(m_logDev, nullptr);
  DestroyDebugReportCallbackEXT(global::GetInstance(), m_callback, nullptr);
  vkDestroyInstance(global::GetInstance(), nullptr);
}


void Base::SetupWindow(uint32_t width, uint32_t height)
{
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
  m_window = glfwCreateWindow(width, height, "PBR Test Vulkan", nullptr, nullptr);
  glfwMakeContextCurrent(m_window);

  glfwSetKeyCallback(m_window, global::KeyCallback);
#if defined(_WIN32)
  //HWND console = GetConsoleWindow();
  //ShowWindow(console, SW_HIDE);
#endif
}


void Base::CloseWindow()
{
  if (m_window) {
    glfwSetWindowShouldClose(m_window, GL_TRUE);
  }
  glfwDestroyWindow(m_window);
}


void Base::SetDebugCallback()
{
  VkDebugReportCallbackCreateInfoEXT createInfo = { };
  createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
  createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
  createInfo.pfnCallback = DebugCallback;

  VkResult result = CreateDebugReportCallbackEXT(
    global::GetInstance(), &createInfo, nullptr, &m_callback);
}


bool IsDeviceSuitable(VkPhysicalDevice device)
{
  // TODO(Garcia): Create a more intelligent 
  // favorability choice maker instead of just going for
  // a blatant discrete gpu. For now, this will do :3
  VkPhysicalDeviceProperties deviceProperties;
  vkGetPhysicalDeviceProperties(device, &deviceProperties);
  VkPhysicalDeviceFeatures deviceFeatures;
  vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
  // Check if the physical device is discrete, and if it has geometry shader support...
  return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && 
          deviceFeatures.geometryShader ;
}


void Base::FindPhyiscalDevice()
{
  uint32_t count = 0;
  vkEnumeratePhysicalDevices(global::GetInstance(),
    &count, nullptr);
  BASE_ASSERT((count > 0) && "Could not find any GPUs with Vulkan support!");
  std::vector<VkPhysicalDevice> physicalDevices(count);
  vkEnumeratePhysicalDevices(global::GetInstance(),
    &count, physicalDevices.data());
  for (const auto &device : physicalDevices) {
    if (IsDeviceSuitable(device)) {
      m_phyDev = device;
      break;
    }
  }
  BASE_ASSERT(m_phyDev != VK_NULL_HANDLE && "Base member m_phyDev is still NULL!!"); 
}


bool Base::QueueFamilyIndices::IsComplete()
{
  return graphicsFamily >= 0 && presentFamily >= 0;
}


Base::QueueFamilyIndices Base::FindQueueFamilies()
{
  QueueFamilyIndices indices;
  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(m_phyDev,
    &queueFamilyCount, nullptr);
  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(m_phyDev,
    &queueFamilyCount, queueFamilies.data());
  uint32_t i = 0;
  for (const auto &queueFamily : queueFamilies) {
    VkBool32 presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(m_phyDev, i, m_surface, &presentSupport);
    if (queueFamily.queueCount > 0 && queueFamily.queueFlags  & VK_QUEUE_GRAPHICS_BIT) {
      indices.graphicsFamily = i;
    }
    if (queueFamily.queueCount > 0 && presentSupport) {
      indices.presentFamily = i;
    }
    if (indices.IsComplete()) {
      break;
    }
    ++i;
  }
  return indices;
}


void Base::CreateLogicalDevice()
{
  QueueFamilyIndices indices = FindQueueFamilies();
  float queuePriority = 1.0f;
  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
  std::set<int32_t> uniqueQueueFamilies = { indices.graphicsFamily, indices.presentFamily };
  for(int32_t queueFamily : uniqueQueueFamilies) {
    VkDeviceQueueCreateInfo queueCreateInfo = { };
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = queueFamily;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;
    queueCreateInfos.push_back(queueCreateInfo);
  }
  
  VkPhysicalDeviceFeatures phyDevFeatures = { };
  VkDeviceCreateInfo deviceCreateInfo = { };
  deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  deviceCreateInfo.queueCreateInfoCount = (uint32_t )queueCreateInfos.size();
  deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
  deviceCreateInfo.pEnabledFeatures = &phyDevFeatures;
  deviceCreateInfo.enabledExtensionCount = 0;
  if (global::enableValidationLayers) {
    deviceCreateInfo.enabledLayerCount = global::validationLayers.size();
    deviceCreateInfo.ppEnabledLayerNames = global::validationLayers.data();
  }
  VkResult result = vkCreateDevice(m_phyDev, &deviceCreateInfo, nullptr, &m_logDev);
  BASE_ASSERT(result == VK_SUCCESS  && "Failed to create a logical device...");

  // Get the device queue.
  vkGetDeviceQueue(m_logDev, indices.graphicsFamily, 0, &m_queue.rendering);
  vkGetDeviceQueue(m_logDev, indices.presentFamily, 0, &m_queue.presentation);
}


void Base::CreateSurface()
{
  VkResult result = glfwCreateWindowSurface(global::GetInstance(),
    m_window, nullptr, &m_surface);
  BASE_ASSERT(result == VK_SUCCESS && "Failed to Create a KHR surface!");
}


void Base::Initialize()
{
  SetDebugCallback();
  CreateSurface();
  FindPhyiscalDevice();
  CreateLogicalDevice(); ;
}


void Base::Run()
{
  while (!glfwWindowShouldClose(m_window)) {
    double t = glfwGetTime();
    m_dt = t - m_lastTime;
    m_lastTime = t;
    glfwPollEvents();
//    std::string str(std::to_string(m_dt) + " delta ms");
//    glfwSetWindowTitle(m_window, str.c_str());

    
    glfwSwapBuffers(m_window); 
  }
  glfwTerminate();
}


void Base::Cleanup()
{
}
} // pbr