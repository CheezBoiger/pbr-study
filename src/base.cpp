//
// Copyright (c) Mario Garcia, MIT License.
//
#include "base.hpp"
#include "shader.hpp"
#include "vertex.hpp"
#include "model.hpp"
#include "geometry.hpp"
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTTION
#include "stb_image.h"

#include <string>
#include <set>
#include <vector>
#include <assert.h>
#include <algorithm>
#include <iostream>
#include <array>
#include <chrono>

#include <gli/gli.hpp>


// For debugging. Set to 1 to start up validation layers.
// You can disable this value to prevent validation layers from
// setting up, which will remove error checking when rendering.
// Although this may sound atrocious, it is an added advantage,
// for which performance may increase as a result of no error checking.
#ifdef _WIN32
 #if _DEBUG 
 #define BASE_DEBUG 1
 #else
  #define BASE_DEBUG 0
 #endif
#else
 #define BASE_DEBUG 1
#endif
#define APPEND_AB(a, b) a##b

// If you prefer to render the sphere, set this to 1 
#define SPHERE 1

#if BASE_DEBUG
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


const std::vector<const char *> validationLayers  { "VK_LAYER_LUNARG_standard_validation" };
const std::vector<const char *> deviceExensions   { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

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
  instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
  instanceCreateInfo.ppEnabledExtensionNames = extensions.data();
  if (enableValidationLayers) {
    instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
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


bool keyCodes[1024];
void KeyCallback(Window window, int key, int scancode, int action, int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  }
  if (action == GLFW_PRESS || action == GLFW_REPEAT) {
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
    keyCodes[key] = true;
  } else {
    keyCodes[key] = false;
  }
}


// Get the Binding Description for the Pipeline.
VkVertexInputBindingDescription GetBindingDescription() 
{
  VkVertexInputBindingDescription bindingDescription = { };
  bindingDescription.stride = sizeof(Vertex);
  bindingDescription.binding = 0;
  bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
  return bindingDescription;
}


// Describe our Vertex Attributes.
std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescriptions()
{
  std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions;
  uint32_t offset = 0;
  attributeDescriptions[0].binding = 0;
  attributeDescriptions[0].location = 0;
  attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
  attributeDescriptions[0].offset = offset;
  offset += sizeof(glm::vec3);
  
  attributeDescriptions[1].binding = 0;
  attributeDescriptions[1].location = 1;
  attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
  attributeDescriptions[1].offset = offset;
  offset += sizeof(glm::vec3);

  attributeDescriptions[2].binding = 0;
  attributeDescriptions[2].location = 2;
  attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
  attributeDescriptions[2].offset = offset;
  offset += sizeof(glm::vec2);

  return attributeDescriptions;
}


// test vertices.
const std::vector<Vertex> vertices = {
  { { -0.5f, -0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
  { {  0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f } },
  { {  0.5f,  0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f } },
  { { -0.5f,  0.5f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f } }
};

const std::vector<uint32_t> indices = {
  0, 1, 2, 2, 3, 0 
}; 

#if SPHERE
const GeometryData model = Geometry::CreateSphere(1.0f, 60, 60);
#else
const GeometryData model = Model::LoadModel("Happy buddha", PBR_STUDY_DIR"/dragon.obj");
#endif

const GeometryData skybox = Geometry::CreateCube();
} // global


// Global Uniform Buffer Object
struct UBO {
  glm::mat4 Model;
  glm::mat4 View;
  glm::mat4 Projection;
  glm::vec3 CamPosition;
} ubo;


struct MaterialUBO {
  float roughness;
  float metallic;
  float gloss;
  float r;
  float g;
  float b;
} material;


struct PointLightUBO {
  glm::vec4 Position;
  glm::vec3 Color;
  float Radius;
  int enable;
} pointLight;


Base::Base()
  : mSwapchain(VK_NULL_HANDLE)
{
  glfwInit();
}


Base::~Base()
{
  CloseWindow();
  Cleanup();
  vkDestroySemaphore(mLogicalDevice, mSemaphores.presentation, nullptr);
  vkDestroySemaphore(mLogicalDevice, mSemaphores.rendering, nullptr);
  // Destroy swapchain image views
  for (size_t i = 0; i < mSwapchainImageViews.size(); ++i) {
    vkDestroyImageView(mLogicalDevice, mSwapchainImageViews[i], nullptr);
    vkDestroyFramebuffer(mLogicalDevice, mSwapchainFramebuffers[i], nullptr);
  }

  vkFreeCommandBuffers(mLogicalDevice, mCommandPool, 
    static_cast<uint32_t>(mCommandBuffers.size()), mCommandBuffers.data());

  vkFreeMemory(mLogicalDevice, mEnvMap.memory, nullptr);
  vkFreeMemory(mLogicalDevice, mDepth.memory, nullptr);
  vkFreeMemory(mLogicalDevice, mMaterial.memory, nullptr);
  vkFreeMemory(mLogicalDevice, mMaterial.stagingMemory, nullptr);
  vkFreeMemory(mLogicalDevice, mPointLight.stagingMemory, nullptr);
  vkFreeMemory(mLogicalDevice, mPointLight.memory, nullptr);
  vkFreeMemory(mLogicalDevice, mSkybox.memory, nullptr);
  vkFreeMemory(mLogicalDevice, mIrradianceMap.memory, nullptr);
  vkFreeMemory(mLogicalDevice, mSkyboxUBO.memory, nullptr);
  vkFreeMemory(mLogicalDevice, mSkyboxUBO.stagingMemory, nullptr);
  vkDestroyBuffer(mLogicalDevice, mMaterial.stagingBuffer, nullptr);
  vkDestroyBuffer(mLogicalDevice, mMaterial.buffer, nullptr);
  vkDestroyBuffer(mLogicalDevice, mPointLight.stagingBuffer, nullptr);
  vkDestroyBuffer(mLogicalDevice, mPointLight.buffer, nullptr);
  vkDestroyBuffer(mLogicalDevice, mSkyboxUBO.buffer, nullptr);
  vkDestroyBuffer(mLogicalDevice, mSkyboxUBO.stagingBuffer, nullptr);
  vkDestroyImage(mLogicalDevice, mEnvMap.image, nullptr);
  vkDestroyImage(mLogicalDevice, texture.image, nullptr);
  vkDestroyImage(mLogicalDevice, mDepth.image, nullptr);
  vkDestroyImage(mLogicalDevice, mSkybox.image, nullptr);
  vkDestroyImage(mLogicalDevice, mIrradianceMap.image, nullptr);
  vkDestroyImageView(mLogicalDevice, mEnvMap.view, nullptr);
  vkDestroyImageView(mLogicalDevice, mDepth.imageView, nullptr);
  vkDestroyImageView(mLogicalDevice, texture.imageView, nullptr);
  vkDestroyImageView(mLogicalDevice, mSkybox.view, nullptr);
  vkDestroyImageView(mLogicalDevice, mIrradianceMap.view, nullptr); 
  vkDestroySampler(mLogicalDevice, mEnvMap.sampler, nullptr);
  vkDestroySampler(mLogicalDevice, texture.sampler, nullptr);
  vkDestroySampler(mLogicalDevice, mSkybox.sampler, nullptr);
  vkDestroySampler(mLogicalDevice, mIrradianceMap.sampler, nullptr);
  vkFreeMemory(mLogicalDevice, texture.memory, nullptr);
  vkFreeMemory(mLogicalDevice, mUbo.stagingMemory, nullptr);
  vkFreeMemory(mLogicalDevice, mUbo.memory, nullptr);
  vkDestroyBuffer(mLogicalDevice, mUbo.buffer, nullptr);
  vkDestroyBuffer(mLogicalDevice, mUbo.stagingBuffer, nullptr);
  vkDestroyDescriptorSetLayout(mLogicalDevice, mDescriptorSetLayout, nullptr);
  vkDestroyDescriptorPool(mLogicalDevice, mDescriptorPool, nullptr);
  vkFreeMemory(mLogicalDevice, mesh.vertexMemory, nullptr);
  vkFreeMemory(mLogicalDevice, mesh.indicesMemory, nullptr);
  vkDestroyBuffer(mLogicalDevice, mesh.indicesBuffer, nullptr);
  vkDestroyBuffer(mLogicalDevice, mesh.vertexBuffer, nullptr);
  vkDestroyCommandPool(mLogicalDevice, mCommandPool, nullptr);
  vkDestroyPipeline(mLogicalDevice, mPipelines.pbr, nullptr);
  vkDestroyPipeline(mLogicalDevice, mPipelines.skybox, nullptr);
  vkDestroyRenderPass(mLogicalDevice, mDefaultRenderPass, nullptr);
  vkDestroyPipelineLayout(mLogicalDevice, mPipelineLayout, nullptr);
  vkDestroySwapchainKHR(mLogicalDevice, mSwapchain, nullptr);
  vkDestroySurfaceKHR(global::GetInstance(), mSurface, nullptr);
  vkDestroyDevice(mLogicalDevice, nullptr);
  DestroyDebugReportCallbackEXT(global::GetInstance(), mCallback, nullptr);
  vkDestroyInstance(global::GetInstance(), nullptr);
}


void Base::OnWindowResized(global::Window window, int width, int height)
{
  if (width == 0 || height == 0) return;
  Base *base = static_cast<Base *>(glfwGetWindowUserPointer(window));
  base->RecreateSwapchain();
}


void Base::SetupWindow(uint32_t width, uint32_t height)
{
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
  mWindow = glfwCreateWindow(width, height, "PBR Test Vulkan", nullptr, nullptr);
  glfwMakeContextCurrent(mWindow);

  // Set the reference to the object that has ownership of this window.
  glfwSetWindowUserPointer(mWindow, this);

  glfwSetKeyCallback(mWindow, global::KeyCallback);
  glfwSetWindowSizeCallback(mWindow, Base::OnWindowResized);
#if defined(_WIN32)
  //HWND console = GetConsoleWindow();
  //ShowWindow(console, SW_HIDE);
#endif
}


Base::SwapChainSupportDetails Base::QuerySwapChainSupport(VkPhysicalDevice device)
{
  SwapChainSupportDetails details;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, mSurface, &details.capabilities);
  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, mSurface, &formatCount, nullptr);
  if (formatCount > 0) {
    details.formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, mSurface, 
      &formatCount, details.formats.data());
  }
  uint32_t presentCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device, mSurface, &presentCount, nullptr);
  if (presentCount > 0) {
    details.presentModes.resize(presentCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, mSurface, 
      &presentCount, details.presentModes.data());
  }

  return details;
}


void Base::CloseWindow()
{
  if (mWindow) {
    glfwSetWindowShouldClose(mWindow, GL_TRUE);
  }
  glfwDestroyWindow(mWindow);
}


void Base::SetDebugCallback()
{
  VkDebugReportCallbackCreateInfoEXT createInfo = { };
  createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
  createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
  createInfo.pfnCallback = DebugCallback;

  VkResult result = CreateDebugReportCallbackEXT(
    global::GetInstance(), &createInfo, nullptr, &mCallback);
}


bool CheckDeviceExensionSupport(VkPhysicalDevice device)
{
  uint32_t extensionsCount;
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionsCount, nullptr);
  std::vector<VkExtensionProperties> availableExtensions(extensionsCount);
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionsCount, availableExtensions.data());
  std::set<std::string> requiredExtensions(global::deviceExensions.begin(), global::deviceExensions.end());
  for (const auto &extension : availableExtensions) {
    requiredExtensions.erase(extension.extensionName);
  }
  return requiredExtensions.empty();
}


bool Base::IsDeviceSuitable(VkPhysicalDevice device)
{
  // TODO(Garcia): Create a more intelligent 
  // favorability choice maker instead of just going for
  // a blatant discrete gpu. For now, this will do :3
  VkPhysicalDeviceProperties deviceProperties;
  vkGetPhysicalDeviceProperties(device, &deviceProperties);
  VkPhysicalDeviceFeatures deviceFeatures;
  vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

  bool extensionsSupported = CheckDeviceExensionSupport(device);
  bool swapChainAdequate = false;
  if (extensionsSupported) {
    SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
    swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
  }
  // Check if the physical device is discrete, and if it is swapchain and surface supported...
  // VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU for intel integrated graphics...
  return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
         extensionsSupported && swapChainAdequate;
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
      mPhysicalDevice = device;
      break;
    }
  }
  BASE_ASSERT(mPhysicalDevice != VK_NULL_HANDLE && "Base member m_phyDev is still NULL!!"); 
}


VkSurfaceFormatKHR Base::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats)
{
  if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED) {
    return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
  }
  for (const auto &availableFormat : availableFormats) {
    if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && 
      availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return availableFormat;
    }
  }
  
  return availableFormats[0];
}


VkPresentModeKHR Base::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes)
{
  VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;
  for (const auto &availablePresentMode : availablePresentModes) {
    if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
      return availablePresentMode;
    } else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
      bestMode = availablePresentMode;
    }
  }
  return bestMode;
}


VkExtent2D Base::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities)
{
  if (capabilities.currentExtent.width != (std::numeric_limits<uint32_t>::max)()) {
    return capabilities.currentExtent;
  } else {
    int32_t width, height;
    glfwGetWindowSize(mWindow, &width, &height);
    VkExtent2D actualExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
    actualExtent.width = (std::max)(capabilities.minImageExtent.width, 
      (std::min)(capabilities.maxImageExtent.width, actualExtent.width));
    actualExtent.height = (std::max)(capabilities.minImageExtent.height, 
      (std::min)(capabilities.minImageExtent.height, actualExtent.height));
    return actualExtent;
  }
}


bool Base::QueueFamilyIndices::IsComplete()
{
  return graphicsFamily >= 0 && presentFamily >= 0;
}


Base::QueueFamilyIndices Base::FindQueueFamilies(VkPhysicalDevice device)
{
  QueueFamilyIndices indices;
  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device,
    &queueFamilyCount, nullptr);
  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(device,
    &queueFamilyCount, queueFamilies.data());
  uint32_t i = 0;
  for (const auto &queueFamily : queueFamilies) {
    VkBool32 presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, mSurface, &presentSupport);
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
  QueueFamilyIndices indices = FindQueueFamilies(mPhysicalDevice);
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
  deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(global::deviceExensions.size());
  deviceCreateInfo.ppEnabledExtensionNames = global::deviceExensions.data();
  if (global::enableValidationLayers) {
    deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(global::validationLayers.size());
    deviceCreateInfo.ppEnabledLayerNames = global::validationLayers.data();
  }
  VkResult result = vkCreateDevice(mPhysicalDevice, &deviceCreateInfo, nullptr, &mLogicalDevice);
  BASE_ASSERT(result == VK_SUCCESS  && "Failed to create a logical device...");

  // Get the device queue.
  vkGetDeviceQueue(mLogicalDevice, indices.graphicsFamily, 0, &mQueues.rendering);
  vkGetDeviceQueue(mLogicalDevice, indices.presentFamily, 0, &mQueues.presentation);
}


void Base::CreateSurface()
{
  VkResult result = glfwCreateWindowSurface(global::GetInstance(),
    mWindow, nullptr, &mSurface);
  BASE_ASSERT(result == VK_SUCCESS && "Failed to Create a KHR surface!");
}


void Base::CreateSwapChain()
{
  SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(mPhysicalDevice);
  VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
  VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
  VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

  uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
  if (swapChainSupport.capabilities.maxImageCount > 0 && 
      imageCount > swapChainSupport.capabilities.maxImageCount) {
    imageCount = swapChainSupport.capabilities.maxImageCount;
  }
  VkSwapchainKHR old_swapchain = mSwapchain;
  VkSwapchainKHR new_swapchain;
  VkSwapchainCreateInfoKHR swapChainCreateInfo = { };
  swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  swapChainCreateInfo.surface = mSurface;
  swapChainCreateInfo.minImageCount = imageCount;
  swapChainCreateInfo.imageFormat = surfaceFormat.format;
  swapChainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
  swapChainCreateInfo.imageExtent = extent;
  swapChainCreateInfo.imageArrayLayers = 1;
  // No offscreen rendering, unless needed. 
  swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; 

  QueueFamilyIndices indices = FindQueueFamilies(mPhysicalDevice);
  uint32_t queueFamilyIndices[] = { (uint32_t )indices.graphicsFamily, (uint32_t )indices.presentFamily };
  if (indices.graphicsFamily != indices.presentFamily) {
    swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    swapChainCreateInfo.queueFamilyIndexCount = 2;
    swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
  } else {
    swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapChainCreateInfo.queueFamilyIndexCount = 0;
    swapChainCreateInfo.pQueueFamilyIndices = nullptr;
  }
  swapChainCreateInfo.preTransform = swapChainSupport.capabilities.currentTransform;
  swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  swapChainCreateInfo.presentMode = presentMode;
  swapChainCreateInfo.clipped = VK_TRUE;
  swapChainCreateInfo.oldSwapchain = old_swapchain;
 
  VkResult result = vkCreateSwapchainKHR(mLogicalDevice, &swapChainCreateInfo, nullptr, &new_swapchain);
  BASE_ASSERT(result == VK_SUCCESS && "Swapchain failed to create!"); 
  mSwapchain = new_swapchain;
  vkGetSwapchainImagesKHR(mLogicalDevice, mSwapchain, &imageCount, nullptr);
  mSwapchainImages.resize(imageCount);
  vkGetSwapchainImagesKHR(mLogicalDevice, mSwapchain, &imageCount, mSwapchainImages.data());

  // Keep track of swapchain format at extent.
  mSwapchainFormat = surfaceFormat.format;
  mSwapchainExtent = extent;

  // If the old swapchain is a null handle, ignore having to delete it.
  if (old_swapchain != VK_NULL_HANDLE) {
    vkDestroySwapchainKHR(mLogicalDevice, old_swapchain, nullptr);
  }
}


void Base::CreateImageViews()
{
  mSwapchainImageViews.resize(mSwapchainImages.size());
  for (uint32_t i = 0; i < mSwapchainImages.size(); ++i) {
    CreateImageView(mSwapchainImages[i], mSwapchainFormat, VK_IMAGE_ASPECT_COLOR_BIT, mSwapchainImageViews[i]);
  }
}


void Base::CreateCubemap(gli::texture_cube &cubeMap, Cubemap &cubemap)
{
  VkBuffer stagingBuffer;
  VkDeviceMemory stagingMemory;

  VkBufferCreateInfo createInfo = { };
  createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  createInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  createInfo.size = cubeMap.size();
  VkResult result = vkCreateBuffer(mLogicalDevice, &createInfo, nullptr, &stagingBuffer);
  BASE_ASSERT(result == VK_SUCCESS && "Failed to create staging buffer");
  
  VkMemoryRequirements memReqs = { };
  vkGetBufferMemoryRequirements(mLogicalDevice, stagingBuffer, &memReqs);
  
  VkMemoryAllocateInfo allocInfo = { };
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.memoryTypeIndex = FindMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
  allocInfo.allocationSize = memReqs.size;
  
  result = vkAllocateMemory(mLogicalDevice, &allocInfo, nullptr, &stagingMemory);
  BASE_ASSERT(result == VK_SUCCESS && "Failed to allocate staging buffer memory!");

  vkBindBufferMemory(mLogicalDevice, stagingBuffer, stagingMemory, 0);
  
  void *data;
  vkMapMemory(mLogicalDevice, stagingMemory, 0, memReqs.size, 0, &data);
    memcpy(data, cubeMap.data(), (size_t )cubeMap.size());
  vkUnmapMemory(mLogicalDevice, stagingMemory);

  std::vector<VkBufferImageCopy> bufferCopyRegions;
  size_t offset = 0;
  for (uint32_t face = 0; face < 6; ++face) {
    for (uint32_t level = 0; level < cubeMap.levels(); ++level) {
      VkBufferImageCopy copyRegion = { };
      copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      copyRegion.imageSubresource.baseArrayLayer = face;
      copyRegion.imageSubresource.mipLevel = level;
      copyRegion.imageSubresource.layerCount = 1;
      copyRegion.imageExtent.width = (uint32_t )cubeMap[face][level].extent().x;
      copyRegion.imageExtent.height = (uint32_t )cubeMap[face][level].extent().y;
      copyRegion.imageExtent.depth = 1;
      copyRegion.bufferOffset = offset;
      bufferCopyRegions.push_back(copyRegion);

      // increase offset.
      offset += cubeMap[face][level].size();
    }
  }

  uint32_t width = cubeMap.extent().x;
  uint32_t height = cubeMap.extent().y;
  // no need for mipmap levels, we don't have any.
  VkImageCreateInfo imageInfo = { };
  imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  imageInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
  imageInfo.imageType = VK_IMAGE_TYPE_2D;
  imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageInfo.mipLevels = cubeMap.levels();
  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
  imageInfo.extent = { width, height, 1 };
  imageInfo.arrayLayers = 6;
  
  result = vkCreateImage(mLogicalDevice, &imageInfo, nullptr, &cubemap.image);
  BASE_ASSERT(result == VK_SUCCESS && "Failed to create Enviroment map image!");

  vkGetImageMemoryRequirements(mLogicalDevice, cubemap.image, &memReqs);
  allocInfo.allocationSize = memReqs.size;
  allocInfo.memoryTypeIndex = FindMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  
  result = vkAllocateMemory(mLogicalDevice, &allocInfo, nullptr, &cubemap.memory);
  BASE_ASSERT(result == VK_SUCCESS && "Failed to allocate Enviroment map memory!");
  vkBindImageMemory(mLogicalDevice, cubemap.image, cubemap.memory, 0);
  
  // One time commandbuffer setting.
  VkCommandBuffer commandbuffer = BeginSingleTimeCommands();
  VkImageSubresourceRange subresourceRange = { };
  subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  subresourceRange.baseMipLevel = 0;
  subresourceRange.layerCount = 6;
  subresourceRange.levelCount = cubeMap.levels();
  
  VkImageMemoryBarrier barrier = { };
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  barrier.srcAccessMask = 0;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.subresourceRange = subresourceRange;
  barrier.image = cubemap.image;
  barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

  vkCmdPipelineBarrier(commandbuffer, 
    VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
    0, 0, VK_NULL_HANDLE, 0, VK_NULL_HANDLE, 1, &barrier);
  
  vkCmdCopyBufferToImage(commandbuffer, stagingBuffer, cubemap.image, 
    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, bufferCopyRegions.size(), bufferCopyRegions.data());

  barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
  barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
  barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

  vkCmdPipelineBarrier(commandbuffer,
    VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
    0, 0, VK_NULL_HANDLE, 0, VK_NULL_HANDLE, 1, &barrier);

  // end and flush the commandbuffer.
  EndSingleTimeCommands(commandbuffer);
  
  vkFreeMemory(mLogicalDevice, stagingMemory, nullptr);
  vkDestroyBuffer(mLogicalDevice, stagingBuffer, nullptr);

  VkSamplerCreateInfo samplerInfo = { };
  samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  samplerInfo.magFilter = VK_FILTER_LINEAR;
  samplerInfo.minFilter = VK_FILTER_LINEAR;
  samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  samplerInfo.mipLodBias = 0.0f;
  samplerInfo.maxAnisotropy = 16;
  samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
  samplerInfo.anisotropyEnable = VK_TRUE;
  samplerInfo.maxLod = 1;
  samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_WHITE;
  result = vkCreateSampler(mLogicalDevice, &samplerInfo, nullptr, &cubemap.sampler);
  BASE_ASSERT(result == VK_SUCCESS && "Filed to create enviroment map sampler!");

  VkImageViewCreateInfo imageVCreate = { };
  imageVCreate.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  imageVCreate.format = VK_FORMAT_R32G32B32A32_SFLOAT;
  imageVCreate.image = cubemap.image;
  imageVCreate.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  imageVCreate.subresourceRange.baseArrayLayer = 0;
  imageVCreate.subresourceRange.baseMipLevel = 0;
  imageVCreate.subresourceRange.layerCount = 6;
  imageVCreate.subresourceRange.levelCount = cubeMap.levels();
  imageVCreate.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
  imageVCreate.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
  imageVCreate.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
  imageVCreate.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
  imageVCreate.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
  result = vkCreateImageView(mLogicalDevice, &imageVCreate, nullptr, &cubemap.view);
  BASE_ASSERT(result == VK_SUCCESS && "Failed to create enviroment map image view!");
}


void Base::CreateCubemaps()
{
  gli::texture_cube cubeMap(gli::load(PBR_STUDY_DIR"/maps/subway_skybox.ktx"));
  gli::texture_cube irradianceMap(gli::load(PBR_STUDY_DIR"/maps/subway_irradiance.ktx"));
  gli::texture_cube skyBox(gli::load(PBR_STUDY_DIR"/maps/subway_skybox.ktx"));

  CreateCubemap(cubeMap, mEnvMap);
  CreateCubemap(irradianceMap, mIrradianceMap);
  CreateCubemap(skyBox, mSkybox);
}


void Base::CreateGraphicsPipeline()
{
  VkShaderModule vert = ShaderModule::GenerateShaderModule(mLogicalDevice, 
    ShaderModule::ssVertShader, PBR_STUDY_DIR"/shaders/test.vert");
  VkShaderModule frag = ShaderModule::GenerateShaderModule(mLogicalDevice,
    ShaderModule::ssFragShader, PBR_STUDY_DIR"/shaders/test.frag");
  VkShaderModule skyVert = ShaderModule::GenerateShaderModule(mLogicalDevice,
    ShaderModule::ssVertShader, PBR_STUDY_DIR"/shaders/skybox.vert");
  VkShaderModule skyFrag = ShaderModule::GenerateShaderModule(mLogicalDevice,
    ShaderModule::ssFragShader, PBR_STUDY_DIR"/shaders/skybox.frag");

  VkPipelineShaderStageCreateInfo vertShaderStageInfo = { };
  vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vertShaderStageInfo.module = vert;
  vertShaderStageInfo.pName = ShaderModule::GetStdEntryPoint();

  VkPipelineShaderStageCreateInfo fragShaderStageInfo = { };
  fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT; 
  fragShaderStageInfo.module = frag;
  fragShaderStageInfo.pName = ShaderModule::GetStdEntryPoint();

  VkPipelineShaderStageCreateInfo shaderInfos[] = { vertShaderStageInfo, fragShaderStageInfo };
  auto binding_description = global::GetBindingDescription();
  auto attribute_descriptions = global::GetAttributeDescriptions();

  VkPipelineVertexInputStateCreateInfo vertexInputStateinfo = { };
  vertexInputStateinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputStateinfo.pVertexAttributeDescriptions = attribute_descriptions.data();
  vertexInputStateinfo.vertexAttributeDescriptionCount = 
    static_cast<uint32_t>(attribute_descriptions.size());
  vertexInputStateinfo.pVertexBindingDescriptions = &binding_description;
  vertexInputStateinfo.vertexBindingDescriptionCount = 1;

  VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo = { };
  inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

  VkPipelineTessellationStateCreateInfo tesseCreateInfo = { };
  tesseCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;  
  tesseCreateInfo.patchControlPoints = 3;
  tesseCreateInfo.pNext = nullptr;
  tesseCreateInfo.flags = 0;

  VkViewport viewport = { };
  viewport.x = viewport.y = 0.0f;
  viewport.width = static_cast<float>(mSwapchainExtent.width);
  viewport.height = static_cast<float>(mSwapchainExtent.height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;  
  
  VkRect2D scissor = { };
  scissor.offset.x = scissor.offset.y = 0;
  scissor.extent = mSwapchainExtent;

  // This can be a dynamic state as well, but we don't need to manually
  // define the viewport state unless we are doing other things besides pbr.
  VkPipelineViewportStateCreateInfo viewportStateCreateInfo = { };
  viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;  
  viewportStateCreateInfo.pViewports = &viewport;
  viewportStateCreateInfo.viewportCount = 1;
  viewportStateCreateInfo.scissorCount = 1;
  viewportStateCreateInfo.pScissors = &scissor; 
 
  VkPipelineRasterizationStateCreateInfo rasterStateCreateInfo = { };
  rasterStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterStateCreateInfo.depthClampEnable = VK_FALSE;
  rasterStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
  rasterStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
  rasterStateCreateInfo.lineWidth = 1.0f;
  rasterStateCreateInfo.cullMode = VK_CULL_MODE_FRONT_BIT;
  rasterStateCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;  
  rasterStateCreateInfo.depthBiasEnable = VK_FALSE;
  rasterStateCreateInfo.depthBiasClamp = 0.0f;
  rasterStateCreateInfo.depthBiasConstantFactor = 0.0f;
  rasterStateCreateInfo.depthBiasSlopeFactor = 0.0f;

  VkPipelineMultisampleStateCreateInfo multisampleCreateInfo = { };
  multisampleCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampleCreateInfo.sampleShadingEnable = VK_FALSE;
  multisampleCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multisampleCreateInfo.minSampleShading = 1.0f;
  multisampleCreateInfo.pSampleMask = nullptr;
  multisampleCreateInfo.alphaToCoverageEnable = VK_FALSE;
  multisampleCreateInfo.alphaToOneEnable = VK_FALSE;

  VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo = { };
  depthStencilCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  depthStencilCreateInfo.depthTestEnable = VK_TRUE;
  depthStencilCreateInfo.depthWriteEnable = VK_TRUE;
  depthStencilCreateInfo.depthBoundsTestEnable  = VK_FALSE;
  depthStencilCreateInfo.minDepthBounds = 0.0;
  depthStencilCreateInfo.maxDepthBounds = 1.0f;
  depthStencilCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
  // we don't need stencil at the moment...
  depthStencilCreateInfo.stencilTestEnable = VK_FALSE;
  depthStencilCreateInfo.front = { };
  depthStencilCreateInfo.back =  { };

  VkPipelineColorBlendAttachmentState colorBlendAttachment = { };
  colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
    VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  colorBlendAttachment.blendEnable = VK_FALSE;
  colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
  colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

  VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = { };
  colorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
  colorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY;
  colorBlendStateCreateInfo.attachmentCount = 1;
  colorBlendStateCreateInfo.pAttachments = &colorBlendAttachment;
  colorBlendStateCreateInfo.blendConstants[0] = 0.0f;
  colorBlendStateCreateInfo.blendConstants[1] = 0.0f;
  colorBlendStateCreateInfo.blendConstants[2] = 0.0f;
  colorBlendStateCreateInfo.blendConstants[3] = 0.0f;


  VkDynamicState dynamicStates[] = {
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_LINE_WIDTH
  };

  VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = { };
  dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamicStateCreateInfo.dynamicStateCount = 2;
  dynamicStateCreateInfo.pDynamicStates = dynamicStates;
  
  VkPipelineLayoutCreateInfo pipelineLayoutCreatInfo = { };
  pipelineLayoutCreatInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutCreatInfo.setLayoutCount = 1;
  pipelineLayoutCreatInfo.pSetLayouts = &mDescriptorSetLayout;
  pipelineLayoutCreatInfo.pushConstantRangeCount = 0;
  pipelineLayoutCreatInfo.pPushConstantRanges = nullptr;

  VkResult result = vkCreatePipelineLayout(mLogicalDevice, 
    &pipelineLayoutCreatInfo, nullptr, &mPipelineLayout);
  BASE_ASSERT(result == VK_SUCCESS);

  // Now create the graphics pipeline.
  VkGraphicsPipelineCreateInfo gPipelineCreateInfo = { };
  gPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  gPipelineCreateInfo.stageCount = 2;
  gPipelineCreateInfo.pStages = shaderInfos;
  gPipelineCreateInfo.pVertexInputState  = &vertexInputStateinfo;
  gPipelineCreateInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
  gPipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
  gPipelineCreateInfo.pRasterizationState = &rasterStateCreateInfo;
  gPipelineCreateInfo.pMultisampleState = &multisampleCreateInfo;
  gPipelineCreateInfo.pDepthStencilState = &depthStencilCreateInfo;
  gPipelineCreateInfo.pColorBlendState = &colorBlendStateCreateInfo;
  gPipelineCreateInfo.pDynamicState = nullptr;
  gPipelineCreateInfo.layout = mPipelineLayout;
  gPipelineCreateInfo.renderPass = mDefaultRenderPass;
  // For deriving from a base pipeline (parent). This is a single pipeline, so
  // we aren't deriving from any other pipeline.
  gPipelineCreateInfo.flags = VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT;
  gPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
  gPipelineCreateInfo.basePipelineIndex = -1;
  

  result = vkCreateGraphicsPipelines(mLogicalDevice, VK_NULL_HANDLE, 1, 
    &gPipelineCreateInfo, nullptr, &mPipelines.pbr);
  BASE_ASSERT(result == VK_SUCCESS && "Failed to create PBR graphics pipeline!");

  vkDestroyShaderModule(mLogicalDevice, vert, nullptr);
  vkDestroyShaderModule(mLogicalDevice, frag, nullptr);

  rasterStateCreateInfo.cullMode = VK_CULL_MODE_FRONT_BIT;
  rasterStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  shaderInfos[0].module = skyVert;
  shaderInfos[1].module = skyFrag;
  gPipelineCreateInfo.flags = VK_PIPELINE_CREATE_DERIVATIVE_BIT;
  gPipelineCreateInfo.basePipelineHandle = mPipelines.pbr;
  
  result = vkCreateGraphicsPipelines(mLogicalDevice, VK_NULL_HANDLE, 1,
    &gPipelineCreateInfo, nullptr, &mPipelines.skybox);
  BASE_ASSERT(result == VK_SUCCESS && "Failed to create skybox pipeline!");

  vkDestroyShaderModule(mLogicalDevice, skyVert, nullptr);
  vkDestroyShaderModule(mLogicalDevice, skyFrag, nullptr);  
}


void Base::CreateRenderPasses()
{
  // This is the actual attachment to be bound to 
  // the renderpass.
  VkAttachmentDescription colorAttachment = { };
  colorAttachment.format = mSwapchainFormat;
  colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentDescription depthAttachment = { };
  depthAttachment.format = FindDepthFormat();
  depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthAttachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  

  // layout (location = 0) reference. Can be used in multiple renderpasses.
  VkAttachmentReference attachmentRef = { };
  attachmentRef.attachment = 0; // attachment location
  attachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkAttachmentReference depthReference = { };
  depthReference.attachment = 1;
  depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  // This subpass describes attachment references, as well as it's binding 
  // point (which is for graphics pipelines, whereas it can also bind to 
  // compute pipelines if explicitly told so) to be used for multiple renderpasses.
  VkSubpassDescription subpass = { };
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &attachmentRef;
  subpass.pDepthStencilAttachment = &depthReference;

  VkSubpassDependency subpassDependency = { };
  subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  subpassDependency.dstSubpass = 0;
  subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  subpassDependency.srcAccessMask = 0;
  subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | 
    VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  

  std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
  VkRenderPassCreateInfo renderpassCreateInfo = { };
  renderpassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderpassCreateInfo.attachmentCount = (uint32_t )attachments.size();
  renderpassCreateInfo.pAttachments = attachments.data();
  renderpassCreateInfo.subpassCount = 1;
  renderpassCreateInfo.pSubpasses = &subpass;
  renderpassCreateInfo.dependencyCount = 1;
  renderpassCreateInfo.pDependencies = &subpassDependency;
  
  VkResult result = vkCreateRenderPass(mLogicalDevice, &renderpassCreateInfo,
    nullptr, &mDefaultRenderPass);
  BASE_ASSERT(result == VK_SUCCESS);
}


void Base::CreateFramebuffers()
{
  mSwapchainFramebuffers.resize(mSwapchainImageViews.size());
  for (size_t i = 0; i < mSwapchainImageViews.size(); ++i) {
    std::array<VkImageView, 2> attachments = {
      mSwapchainImageViews[i],
      mDepth.imageView
    };
    VkFramebufferCreateInfo framebufferCreateInfo = { };
    framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferCreateInfo.renderPass = mDefaultRenderPass;
    framebufferCreateInfo.attachmentCount = (uint32_t )attachments.size();
    framebufferCreateInfo.pAttachments = attachments.data();
    framebufferCreateInfo.width = mSwapchainExtent.width;
    framebufferCreateInfo.height = mSwapchainExtent.height;
    framebufferCreateInfo.layers = 1;
    VkResult result = vkCreateFramebuffer(mLogicalDevice, &framebufferCreateInfo, 
      nullptr, &mSwapchainFramebuffers[i]);
    BASE_ASSERT(result == VK_SUCCESS && "A Swapchain Framebuffer failed to create!");
  }
}


void Base::CreateCommandPool()
{
  QueueFamilyIndices indices = FindQueueFamilies(mPhysicalDevice);
  VkCommandPoolCreateInfo commandPoolCreateInfo = { };
  commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  commandPoolCreateInfo.queueFamilyIndex = indices.graphicsFamily;
  commandPoolCreateInfo.flags = 0;
  VkResult result = vkCreateCommandPool(mLogicalDevice, &commandPoolCreateInfo, nullptr, &mCommandPool);
  BASE_ASSERT(result == VK_SUCCESS && "Failed to create a command pool!");
}


void Base::CreateCommandBuffers()
{
  mCommandBuffers.resize(mSwapchainFramebuffers.size());

  {
    VkCommandBufferAllocateInfo cmdAllocInfo = { };
    cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdAllocInfo.commandPool = mCommandPool;
    cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdAllocInfo.commandBufferCount = static_cast<uint32_t>(mCommandBuffers.size());
    VkResult result = vkAllocateCommandBuffers(mLogicalDevice, &cmdAllocInfo, mCommandBuffers.data());
    BASE_ASSERT(result == VK_SUCCESS && "Failed to allocated commandbuffers!");
  }

  for (size_t i = 0; i < mCommandBuffers.size(); ++i) {
    VkCommandBufferBeginInfo cmdBeginInfo = { };
    cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBeginInfo.pInheritanceInfo = nullptr;
    cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    cmdBeginInfo.pNext = nullptr;
    vkBeginCommandBuffer(mCommandBuffers[i], &cmdBeginInfo);    
    
    VkRenderPassBeginInfo renderpassBegin = { };
    renderpassBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderpassBegin.renderPass = mDefaultRenderPass;
    renderpassBegin.framebuffer = mSwapchainFramebuffers[i];
    renderpassBegin.renderArea.offset = { 0, 0 };
    renderpassBegin.renderArea.extent = mSwapchainExtent;
    std::array<VkClearValue, 2> clearValues;
    clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
    clearValues[1].depthStencil = { 1.0f, 0 };
    renderpassBegin.clearValueCount = (uint32_t )clearValues.size();
    renderpassBegin.pClearValues = clearValues.data();
    vkCmdBeginRenderPass(mCommandBuffers[i], &renderpassBegin, VK_SUBPASS_CONTENTS_INLINE);
    VkDeviceSize offsets[] = { 0 };

    vkCmdBindPipeline(mCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelines.skybox);
    vkCmdBindDescriptorSets(mCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout, 
      0, 1, &mDescriptorSetSkybox, 0, nullptr);
    vkCmdBindVertexBuffers(mCommandBuffers[i], 0, 1, &mesh.vertexBuffer, offsets);
    vkCmdBindIndexBuffer(mCommandBuffers[i], mesh.indicesBuffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(mCommandBuffers[i], (uint32_t )global::model.indices.size(), 1, 0, 0, 0);

    
    vkCmdBindPipeline(mCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelines.pbr);
    vkCmdBindDescriptorSets(mCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout, 0,
      1, &mDescriptorSet, 0, nullptr);
    vkCmdBindVertexBuffers(mCommandBuffers[i], 0, 1, &mesh.vertexBuffer, offsets);
    vkCmdBindIndexBuffer(mCommandBuffers[i], mesh.indicesBuffer, 0, VK_INDEX_TYPE_UINT32);
    // hardcoded values.
    //vkCmdDraw(m_commandbuffers[i], (uint32_t )global::vertices.size(), 1, 0, 0);
    vkCmdDrawIndexed(mCommandBuffers[i], (uint32_t )global::model.indices.size(), 1, 0, 0, 0);
    vkCmdEndRenderPass(mCommandBuffers[i]);
    VkResult result = vkEndCommandBuffer(mCommandBuffers[i]);
    BASE_ASSERT(result == VK_SUCCESS && "A CommandBuffer failed recording!");
  }
}


void Base::CreateSemaphores()
{
  VkSemaphoreCreateInfo semaphoreCreateInfo = { };
  semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  VkResult result = vkCreateSemaphore(mLogicalDevice, &semaphoreCreateInfo, 
    nullptr, &mSemaphores.presentation);
  BASE_ASSERT(result == VK_SUCCESS && "Failed to create presentation semaphore!");
  result = vkCreateSemaphore(mLogicalDevice, &semaphoreCreateInfo,
    nullptr, &mSemaphores.rendering);
  BASE_ASSERT(result == VK_SUCCESS && "Failed to create rendering semaphore!");
}


uint32_t Base::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
  VkPhysicalDeviceMemoryProperties memProps = { };
  vkGetPhysicalDeviceMemoryProperties(mPhysicalDevice, &memProps);
  for (uint32_t i = 0; i < memProps.memoryTypeCount; ++i) {
    if (typeFilter & (1 << i) && 
        (memProps.memoryTypes[i].propertyFlags & properties) == properties) {
      return i;
    }
  }
  return -1;
}


void Base::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
  VkBuffer &buffer, VkDeviceMemory &bufferMem)
{
  VkBufferCreateInfo bufferInfo = {};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = size;
  bufferInfo.usage = usage;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  bufferInfo.flags = 0;
  VkResult result = vkCreateBuffer(mLogicalDevice, &bufferInfo, nullptr, &buffer);
  BASE_ASSERT(result == VK_SUCCESS && "Failed to create vertex buffer!");
  VkMemoryRequirements memReqs = {};
  vkGetBufferMemoryRequirements(mLogicalDevice, buffer, &memReqs);
  VkMemoryAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memReqs.size;
  allocInfo.memoryTypeIndex = FindMemoryType(memReqs.memoryTypeBits, properties);
  result = vkAllocateMemory(mLogicalDevice, &allocInfo, nullptr, &bufferMem);
  BASE_ASSERT(result == VK_SUCCESS && "Failed to Create Vertex buffer memory");
  vkBindBufferMemory(mLogicalDevice, buffer, bufferMem, 0);
}


void Base::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
  VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

  VkBufferCopy copyRegion = { };
  copyRegion.size = size;
  vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

  EndSingleTimeCommands(commandBuffer);
}


void Base::CreateVertexBuffers()
{
  VkDeviceSize bufferSize = sizeof(global::model.vertices[0]) * global::model.vertices.size();
  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMem;  
  CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMem);

  // map memory to vertexbuffer.  
  void *data;
  vkMapMemory(mLogicalDevice, stagingBufferMem, 0, bufferSize, 0, &data);
    memcpy(data, global::model.vertices.data(), (size_t )bufferSize);
  vkUnmapMemory(mLogicalDevice, stagingBufferMem);

  CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
    mesh.vertexBuffer, mesh.vertexMemory);

  CopyBuffer(stagingBuffer, mesh.vertexBuffer, bufferSize);

  vkFreeMemory(mLogicalDevice, stagingBufferMem, nullptr);
  vkDestroyBuffer(mLogicalDevice, stagingBuffer, nullptr);
}


void Base::CreateIndexBuffers()
{
  VkDeviceSize bufferSize = sizeof(global::model.indices[0]) * global::model.indices.size();
  VkBuffer stagingBuffer;
  VkDeviceMemory stagingMemory;
  
  CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingMemory);

  void *data;
  vkMapMemory(mLogicalDevice, stagingMemory, 0, bufferSize, 0, &data);
    memcpy(data, global::model.indices.data(), (size_t )bufferSize);
  vkUnmapMemory(mLogicalDevice, stagingMemory);

  CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mesh.indicesBuffer,
    mesh.indicesMemory);

  CopyBuffer(stagingBuffer, mesh.indicesBuffer, bufferSize);

  vkFreeMemory(mLogicalDevice, stagingMemory, nullptr);
  vkDestroyBuffer(mLogicalDevice, stagingBuffer, nullptr);
}


void Base::CreateDescriptorSetLayouts()
{
  std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings;
  VkDescriptorSetLayoutBinding uboLayoutBinding = { };
  uboLayoutBinding.binding = 0;
  uboLayoutBinding.descriptorCount = 1;
  uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  // can be reference in all stages...
  uboLayoutBinding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;

  setLayoutBindings.push_back(uboLayoutBinding);

  VkDescriptorSetLayoutBinding radianceLayoutBinding = { };
  radianceLayoutBinding.binding = 1;
  radianceLayoutBinding.descriptorCount = 1;
  radianceLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  radianceLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  setLayoutBindings.push_back(radianceLayoutBinding);

  VkDescriptorSetLayoutBinding irradianceLayoutBinding = { };
  irradianceLayoutBinding.binding = 2;
  irradianceLayoutBinding.descriptorCount = 1;
  irradianceLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  irradianceLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  setLayoutBindings.push_back(irradianceLayoutBinding);

  VkDescriptorSetLayoutBinding skyboxLayoutBinding = {};
  skyboxLayoutBinding.binding = 3;
  skyboxLayoutBinding.descriptorCount = 1;
  skyboxLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  skyboxLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  setLayoutBindings.push_back(skyboxLayoutBinding);

  VkDescriptorSetLayoutBinding materialLayoutBinding = { };
  materialLayoutBinding.binding = 4;
  materialLayoutBinding.descriptorCount = 1;
  materialLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  materialLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  setLayoutBindings.push_back(materialLayoutBinding);
  
  VkDescriptorSetLayoutBinding lightLayoutBinding = { };
  lightLayoutBinding.binding = 5;
  lightLayoutBinding.descriptorCount = 1;
  lightLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  lightLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  setLayoutBindings.push_back(lightLayoutBinding);
  
  VkDescriptorSetLayoutCreateInfo createInfo = { };
  createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  createInfo.bindingCount = (uint32_t )setLayoutBindings.size();
  createInfo.pBindings = setLayoutBindings.data();
  VkResult result = vkCreateDescriptorSetLayout(mLogicalDevice, &createInfo, nullptr,
    &mDescriptorSetLayout);
  BASE_ASSERT(result == VK_SUCCESS && "Failed to create a descriptor set for ubo.");
}


void Base::CreateUniformBuffers()
{
  VkDeviceSize bufferSize = sizeof(ubo);
  CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, mUbo.stagingBuffer,
    mUbo.stagingMemory);
  CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mUbo.buffer, mUbo.memory);

  bufferSize = sizeof(PointLightUBO);
  CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, mPointLight.stagingBuffer,
    mPointLight.stagingMemory);
  CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mPointLight.buffer,
    mPointLight.memory);

  bufferSize = sizeof(MaterialUBO);
  CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, mMaterial.stagingBuffer,
    mMaterial.stagingMemory);
  CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mMaterial.buffer,
    mMaterial.memory);

  bufferSize = sizeof(ubo);
  CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, mSkyboxUBO.stagingBuffer,
    mSkyboxUBO.stagingMemory);
  CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mSkyboxUBO.buffer,
    mSkyboxUBO.memory);
}


void Base::CreateDescriptorPools()
{
  std::vector<VkDescriptorPoolSize> poolSizes;

  VkDescriptorPoolSize poolSize = { };
  poolSize.descriptorCount = 6;
  poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

  VkDescriptorPoolSize cubemapPool = { };
  cubemapPool.descriptorCount = 8;
  cubemapPool.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
/*
  VkDescriptorPoolSize materialPool = { };
  materialPool.descriptorCount = 1;
  materialPool.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  
  VkDescriptorPoolSize lightPool = { };
  lightPool.descriptorCount = 1;
  lightPool.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
*/
  poolSizes.push_back(poolSize);
  poolSizes.push_back(cubemapPool);
  //poolSizes.push_back(materialPool);
  //poolSizes.push_back(lightPool);

  VkDescriptorPoolCreateInfo poolCreateInfo = { };
  poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolCreateInfo.poolSizeCount = (uint32_t )poolSizes.size();
  poolCreateInfo.pPoolSizes = poolSizes.data();
  poolCreateInfo.maxSets = 2;

  VkResult result = vkCreateDescriptorPool(mLogicalDevice, &poolCreateInfo, nullptr, &mDescriptorPool);
  BASE_ASSERT(result == VK_SUCCESS && "Failed to create ubo descriptor pool");
  
}


void Base::CreateDescriptorSets()
{
  std::array<VkDescriptorSetLayout, 1> layouts = { mDescriptorSetLayout };
  VkDescriptorSetAllocateInfo allocInfo = { };
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorSetCount = 1;
  allocInfo.descriptorPool = mDescriptorPool;
  allocInfo.pSetLayouts = layouts.data();
  VkResult result = vkAllocateDescriptorSets(mLogicalDevice, &allocInfo, &mDescriptorSet);
  BASE_ASSERT(result == VK_SUCCESS && "Failed to alloc descriptor pool on the gpu.");
  result = vkAllocateDescriptorSets(mLogicalDevice, &allocInfo, &mDescriptorSetSkybox);
  BASE_ASSERT(result == VK_SUCCESS && "Failed to alloc descriptor pool for skybox on the gpu.");
  
  // Create the actual descriptor set.
  VkDescriptorBufferInfo bufferInfo = { };
  bufferInfo.buffer = mUbo.buffer;
  bufferInfo.offset = 0;
  bufferInfo.range = sizeof(ubo);

  VkDescriptorImageInfo imageInfo = { };
  imageInfo.sampler = mEnvMap.sampler;
  imageInfo.imageView = mEnvMap.view;
  imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

  VkDescriptorImageInfo irradianceInfo = { };
  irradianceInfo.sampler = mIrradianceMap.sampler;
  irradianceInfo.imageView = mIrradianceMap.view;
  irradianceInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

  VkDescriptorImageInfo skyboxInfo = {};
  skyboxInfo.sampler = mSkybox.sampler;
  skyboxInfo.imageView = mSkybox.view;
  skyboxInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

  VkDescriptorBufferInfo materialBufferInfo = { };
  materialBufferInfo.buffer = mMaterial.buffer;
  materialBufferInfo.offset = 0;
  materialBufferInfo.range = sizeof(material);

  VkDescriptorBufferInfo lightBufferInfo = { };
  lightBufferInfo.buffer = mPointLight.buffer;
  lightBufferInfo.offset = 0;
  lightBufferInfo.range = sizeof(pointLight);

  std::vector<VkWriteDescriptorSet> writeDescriptorSets;  
  // Write to the actual Descriptor set.
  VkWriteDescriptorSet descriptorWrite = { };
  descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptorWrite.dstSet = mDescriptorSet;
  descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  descriptorWrite.dstBinding = 0;
  descriptorWrite.dstArrayElement = 0;
  descriptorWrite.descriptorCount = 1;
  descriptorWrite.pBufferInfo = &bufferInfo;
  descriptorWrite.pImageInfo = nullptr;
  descriptorWrite.pTexelBufferView = nullptr;
  
  VkWriteDescriptorSet cubemapWrite = { };
  cubemapWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  cubemapWrite.dstSet = mDescriptorSet;
  cubemapWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  cubemapWrite.dstBinding = 1;
  cubemapWrite.dstArrayElement = 0;
  cubemapWrite.descriptorCount = 1;
  cubemapWrite.pBufferInfo = nullptr;
  cubemapWrite.pImageInfo = &imageInfo;
  cubemapWrite.pTexelBufferView = nullptr;
  
  VkWriteDescriptorSet irradianceWrite = { };
  irradianceWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  irradianceWrite.dstSet = mDescriptorSet;
  irradianceWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  irradianceWrite.dstBinding = 2;
  irradianceWrite.dstArrayElement = 0;
  irradianceWrite.descriptorCount = 1;
  irradianceWrite.pBufferInfo = nullptr;
  irradianceWrite.pImageInfo = &irradianceInfo;
  irradianceWrite.pTexelBufferView = nullptr;

  VkWriteDescriptorSet skyboxWrite = {};
  skyboxWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  skyboxWrite.dstSet = mDescriptorSet;
  skyboxWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  skyboxWrite.dstBinding = 3;
  skyboxWrite.dstArrayElement = 0;
  skyboxWrite.descriptorCount = 1;
  skyboxWrite.pBufferInfo = nullptr;
  skyboxWrite.pImageInfo = &skyboxInfo;
  skyboxWrite.pTexelBufferView = nullptr;

  VkWriteDescriptorSet materialWrite = { };
  materialWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  materialWrite.dstSet = mDescriptorSet;
  materialWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  materialWrite.dstBinding = 4;
  materialWrite.dstArrayElement = 0;
  materialWrite.descriptorCount = 1;
  materialWrite.pBufferInfo = &materialBufferInfo;

  VkWriteDescriptorSet lightWrite = { };
  lightWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  lightWrite.dstBinding = 5;
  lightWrite.descriptorCount = 1;
  lightWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  lightWrite.dstSet = mDescriptorSet;
  lightWrite.dstArrayElement = 0;
  lightWrite.pBufferInfo = &lightBufferInfo;

  writeDescriptorSets.push_back(descriptorWrite);
  writeDescriptorSets.push_back(cubemapWrite);
  writeDescriptorSets.push_back(irradianceWrite);
  writeDescriptorSets.push_back(skyboxWrite);
  writeDescriptorSets.push_back(materialWrite);
  writeDescriptorSets.push_back(lightWrite);
  
  vkUpdateDescriptorSets(mLogicalDevice, (uint32_t )writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);

  VkDescriptorBufferInfo bufferInfoSky = {};
  bufferInfoSky.buffer = mSkyboxUBO.buffer;
  bufferInfoSky.offset = 0;
  bufferInfoSky.range = sizeof(UBO);

  VkWriteDescriptorSet descriptorWriteSky = {};
  descriptorWriteSky.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptorWriteSky.dstSet = mDescriptorSetSkybox;
  descriptorWriteSky.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  descriptorWriteSky.dstBinding = 0;
  descriptorWriteSky.dstArrayElement = 0;
  descriptorWriteSky.descriptorCount = 1;
  descriptorWriteSky.pBufferInfo = &bufferInfoSky;
  descriptorWriteSky.pImageInfo = nullptr;
  descriptorWriteSky.pTexelBufferView = nullptr;
  writeDescriptorSets.push_back(descriptorWriteSky);

  writeDescriptorSets[0] = descriptorWriteSky;
  writeDescriptorSets[3].dstSet = mDescriptorSetSkybox;

  vkUpdateDescriptorSets(mLogicalDevice, (uint32_t )writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
  writeDescriptorSets.clear();
}


void Base::CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
  VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &imageMemory)
{
  VkImageCreateInfo imageCreateInfo = {};
  imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
  imageCreateInfo.extent.width = width;
  imageCreateInfo.extent.height = height;
  imageCreateInfo.extent.depth = 1;
  imageCreateInfo.mipLevels = 1;
  imageCreateInfo.arrayLayers = 1;
  imageCreateInfo.format = format;
  imageCreateInfo.tiling = tiling;
  // VK_IMAGE_LAYOUT_UNDEFINED for render targets.
  imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
  imageCreateInfo.usage = usage;
  imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageCreateInfo.flags = 0;
  VkResult result = vkCreateImage(mLogicalDevice, &imageCreateInfo, nullptr, &image);
  BASE_ASSERT(result == VK_SUCCESS && "Failed to create staging image.");

  VkMemoryRequirements memReqs = {};
  vkGetImageMemoryRequirements(mLogicalDevice, image, &memReqs);
  VkMemoryAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memReqs.size;
  allocInfo.memoryTypeIndex = FindMemoryType(memReqs.memoryTypeBits, properties);

  result = vkAllocateMemory(mLogicalDevice, &allocInfo, nullptr, &imageMemory);
  BASE_ASSERT(result == VK_SUCCESS && "Failed to allocate stage image memory.");
  vkBindImageMemory(mLogicalDevice, image, imageMemory, 0);
}


void Base::CreateTextureImages()
{
  int32_t width, height, channels;
  stbi_uc *bytecode = stbi_load(PBR_STUDY_DIR"/statue.jpg", &width, &height, &channels, STBI_rgb_alpha);
  VkDeviceSize imageSize = width * height * 4;
  
  BASE_ASSERT(bytecode && "Failed to load image");
  VkImage stagingImage;
  VkDeviceMemory stageMemory;

  CreateImage(width, height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_LINEAR,
    VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    stagingImage, stageMemory);

  VkImageSubresource subresource = {};
  subresource.arrayLayer = 0;
  subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  subresource.mipLevel = 0;
  VkSubresourceLayout stagingImageLayout;
  vkGetImageSubresourceLayout(mLogicalDevice, stagingImage, &subresource, &stagingImageLayout);

  void *data;
  vkMapMemory(mLogicalDevice, stageMemory, 0, imageSize, 0, &data);
    if (stagingImageLayout.rowPitch == width * 4) {
      memcpy(data, bytecode, (size_t )imageSize);
    } else {
      uint8_t *bytes  = static_cast<uint8_t *>(data);
      for (int32_t y = 0; y < height; ++y) {
        memcpy(&bytes[y * stagingImageLayout.rowPitch],
              &bytecode[y * width * 4],
              width * 4);
      }
    }
  vkUnmapMemory(mLogicalDevice, stageMemory);

  CreateImage(width, height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_LINEAR,
    VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, texture.image, texture.memory);  

  TransitionImageLayout(stagingImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_PREINITIALIZED,
    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
  TransitionImageLayout(texture.image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_PREINITIALIZED,
    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);  
  CopyImage(stagingImage, texture.image, width, height);

  TransitionImageLayout(texture.image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);


  vkFreeMemory(mLogicalDevice, stageMemory, nullptr);
  vkDestroyImage(mLogicalDevice, stagingImage, nullptr);
  stbi_image_free(bytecode);
}


VkCommandBuffer Base::BeginSingleTimeCommands()
{
  VkCommandBufferAllocateInfo allocInfo = { };
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;  
  allocInfo.commandPool = mCommandPool;
  allocInfo.commandBufferCount = 1;

  VkCommandBuffer commandbuffer;
  vkAllocateCommandBuffers(mLogicalDevice, &allocInfo, &commandbuffer);
  
  VkCommandBufferBeginInfo beginInfo = { };
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  vkBeginCommandBuffer(commandbuffer, &beginInfo);

  return commandbuffer;
}


void Base::EndSingleTimeCommands(VkCommandBuffer commandbuffer)
{
  vkEndCommandBuffer(commandbuffer);
  
  VkSubmitInfo submitInfo = { };
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandbuffer;
  
  vkQueueSubmit(mQueues.rendering, 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(mQueues.rendering);

  vkFreeCommandBuffers(mLogicalDevice, mCommandPool, 1, &commandbuffer);
}


void Base::CopyImage(VkImage srcImage, VkImage dstImage, uint32_t width, uint32_t height)
{
  VkCommandBuffer commandbuffer = BeginSingleTimeCommands();
  VkImageSubresourceLayers subresource = { };
  subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  subresource.baseArrayLayer = 0;
  subresource.layerCount = 1;
  subresource.mipLevel = 0;

  VkImageCopy region = { };
  region.dstSubresource = subresource;
  region.srcSubresource = subresource;
  region.srcOffset = { 0, 0, 0 };
  region.dstOffset = { 0, 0, 0 };
  region.extent.width = width;
  region.extent.height = height;
  region.extent.depth = 1;
  vkCmdCopyImage(commandbuffer, srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
    dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

  EndSingleTimeCommands(commandbuffer);
}


bool Base::HasStencilComponent(VkFormat format)
{
  return (format == VK_FORMAT_D32_SFLOAT_S8_UINT) || (format == VK_FORMAT_D24_UNORM_S8_UINT);
}


void Base::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout,
  VkImageLayout newLayout)
{
  VkCommandBuffer commandbuffer = BeginSingleTimeCommands();
  
  VkImageMemoryBarrier barrier = { };
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = oldLayout;
  barrier.newLayout = newLayout;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image = image;
  if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    if (HasStencilComponent(format)) {
      barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }
  } else {
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  }
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.layerCount = 1;
  barrier.subresourceRange.levelCount = 1;
  barrier.srcAccessMask = 0;
  barrier.dstAccessMask = 0;
  // too specific! but it matters!
  if (oldLayout == VK_IMAGE_LAYOUT_PREINITIALIZED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
  } else if (oldLayout == VK_IMAGE_LAYOUT_PREINITIALIZED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
  } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
  } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = (VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);
  } else {
    BASE_ASSERT(false && "Failed to modify image barrier access masks.");
  }
  // place the image barrier in the pipeline.
  vkCmdPipelineBarrier(commandbuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr,
    1, &barrier);
  
  EndSingleTimeCommands(commandbuffer);
}


void Base::CreateImageView(VkImage image, VkFormat format, 
  VkImageAspectFlags aspectFlags, VkImageView &imageView)
{
  VkImageViewCreateInfo imageViewCreateInfo = {};
  imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  imageViewCreateInfo.image = image;
  imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  imageViewCreateInfo.format = format;
  imageViewCreateInfo.subresourceRange.aspectMask = aspectFlags;
  imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
  imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
  imageViewCreateInfo.subresourceRange.layerCount = 1;
  imageViewCreateInfo.subresourceRange.levelCount = 1;
  VkResult result = vkCreateImageView(mLogicalDevice, &imageViewCreateInfo, nullptr, &imageView);
  BASE_ASSERT(result == VK_SUCCESS && "Failed to create texture image view.");
}


void Base::CreateTextureSampler()
{
  VkSamplerCreateInfo samplerCreateInfo = { };
  samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
  samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
  samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerCreateInfo.anisotropyEnable = VK_TRUE;
  samplerCreateInfo.maxAnisotropy = 16;
  samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
  samplerCreateInfo.compareEnable = VK_TRUE;
  samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
  samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  samplerCreateInfo.mipLodBias = 0.0f;
  samplerCreateInfo.minLod = 0.0f;
  samplerCreateInfo.maxLod = 0.0f;
  
  VkResult result = vkCreateSampler(mLogicalDevice, &samplerCreateInfo, nullptr, &texture.sampler);
  BASE_ASSERT(result == VK_SUCCESS && "Failed to create the texture sampler!");
}


void Base::CreateTextureImageView()
{
  CreateImageView(texture.image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, texture.imageView);
}


void Base::CreateDefaultDepthResources()
{
  VkFormat depthFormat = FindDepthFormat();
  CreateImage(mSwapchainExtent.width, mSwapchainExtent.height, 
    depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mDepth.image, mDepth.memory);
  CreateImageView(mDepth.image, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, mDepth.imageView);
  TransitionImageLayout(mDepth.image, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, 
    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}


VkFormat Base::FindDepthFormat()
{
  return FindSupportedFormat( {
    VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT
  }, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}


VkFormat Base::FindSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling,
  VkFormatFeatureFlags flags)
{
  for (VkFormat format : candidates) {
    VkFormatProperties properties;
    vkGetPhysicalDeviceFormatProperties(mPhysicalDevice, format, &properties);
    if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & flags) == flags) {
      return format;
    } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & flags) == flags) {
      return format;
    }
  }
  return VK_FORMAT_UNDEFINED;
}


void Base::Initialize()
{
  SetDebugCallback();
  CreateSurface();
  FindPhyiscalDevice();
  CreateLogicalDevice();
  CreateSwapChain();
  CreateImageViews();
  CreateRenderPasses();
  CreateDescriptorSetLayouts();
  CreateGraphicsPipeline();
  CreateCommandPool();
  CreateDefaultDepthResources();
  CreateFramebuffers();
  CreateTextureImages();
  CreateTextureImageView();
  CreateTextureSampler();
  CreateCubemaps();
  CreateVertexBuffers();
  CreateIndexBuffers();
  CreateUniformBuffers();
  CreateDescriptorPools();
  CreateDescriptorSets();
  CreateCommandBuffers();
  CreateSemaphores();
  SetupCamera();

  material.roughness = 0.5f;
  material.metallic = 0.5f;
  material.gloss = 0.0f;
  material.r = 1.0f;
  material.g = 1.0f;
  material.b = 1.0f;
  pointLight.enable = 0;
}


void Base::SetupCamera()
{
  // NOTE(): Gimbal lock if vec3(0.0, x.x, 0.0f))
  mCamera.SetPosition(glm::vec3(2.0f, 2.0f, 2.0f));
  mCamera.SetFov(45.0f);
  mCamera.SetNearFar(0.1f, 1000.0f);
  mCamera.SetSpeed(5.0f);
  mCamera.SetAspect(((float )mSwapchainExtent.width / (float )mSwapchainExtent.height));
  mCamera.SetLookAt(glm::vec3(0.0f, 0.0f, 0.0f));
}


void Base::Draw()
{
  uint32_t imageIndex;
  vkAcquireNextImageKHR(mLogicalDevice, mSwapchain, (std::numeric_limits<uint64_t>::max)(),
    mSemaphores.presentation, VK_NULL_HANDLE, &imageIndex);
  VkSubmitInfo submitInfo = { };
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  VkSemaphore wait_semaphores[] = { mSemaphores.presentation };
  VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = wait_semaphores;
  submitInfo.pWaitDstStageMask = wait_stages;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &mCommandBuffers[imageIndex];
  
  VkSemaphore signal_semaphores[] = { mSemaphores.rendering };
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = signal_semaphores;
  VkResult result = vkQueueSubmit(mQueues.rendering, 1, &submitInfo, VK_NULL_HANDLE); 
  BASE_ASSERT(result == VK_SUCCESS && "Failed to submit commandbuffer to rendering queue.");

  VkPresentInfoKHR presentInfo = { };
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = signal_semaphores;
  VkSwapchainKHR swapchains[] = { mSwapchain };
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = swapchains;
  presentInfo.pImageIndices = &imageIndex;
  presentInfo.pResults = nullptr;
  vkQueuePresentKHR(mQueues.presentation, &presentInfo);
}


void Base::RecreateSwapchain()
{
  vkDeviceWaitIdle(mLogicalDevice);

  for (size_t i = 0; i < mSwapchainImageViews.size(); ++i) {
    vkDestroyImageView(mLogicalDevice, mSwapchainImageViews[i], nullptr);
    vkDestroyFramebuffer(mLogicalDevice, mSwapchainFramebuffers[i], nullptr);
  }

  vkFreeCommandBuffers(mLogicalDevice, mCommandPool,
    static_cast<uint32_t>(mCommandBuffers.size()), mCommandBuffers.data());

  vkDestroyPipeline(mLogicalDevice, mPipelines.pbr, nullptr);
  vkDestroyPipeline(mLogicalDevice, mPipelines.skybox, nullptr);
  vkDestroyRenderPass(mLogicalDevice, mDefaultRenderPass, nullptr);
  vkDestroyPipelineLayout(mLogicalDevice, mPipelineLayout, nullptr);
  vkFreeMemory(mLogicalDevice, mDepth.memory, nullptr);
  vkDestroyImage(mLogicalDevice, mDepth.image, nullptr);
  vkDestroyImageView(mLogicalDevice, mDepth.imageView, nullptr);

  CreateSwapChain();
  CreateImageViews();
  CreateRenderPasses();
  CreateGraphicsPipeline();
  CreateDefaultDepthResources();
  CreateFramebuffers();
  CreateCommandBuffers();
}


void Base::UpdateUniformBuffers()
{
  static auto startTime = std::chrono::high_resolution_clock::now();
  auto currentTime = std::chrono::high_resolution_clock::now();
  float time = std::chrono::duration_cast<std::chrono::milliseconds>(
    currentTime - startTime).count() / 1000.0f;

  ubo.Projection = mCamera.GetProjection();
  // flip projection, vulkan handles everything differently than OpenGL
  ubo.Projection[1][1] *= -1;
  ubo.View = mCamera.GetView();
  ubo.Model = glm::rotate(glm::mat4(), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
  ubo.CamPosition = mCamera.GetPosition();
  void *data;
  vkMapMemory(mLogicalDevice, mUbo.stagingMemory, 0, sizeof(ubo), 0, &data);
    memcpy(data, &ubo, sizeof(ubo));
  vkUnmapMemory(mLogicalDevice, mUbo.stagingMemory);

  CopyBuffer(mUbo.stagingBuffer, mUbo.buffer, sizeof(ubo));

  // update pbr settings.

  vkMapMemory(mLogicalDevice, mMaterial.stagingMemory, 0, sizeof(material), 0, &data);
    memcpy(data, &material, sizeof(material));
  vkUnmapMemory(mLogicalDevice, mMaterial.stagingMemory);
  CopyBuffer(mMaterial.stagingBuffer, mMaterial.buffer, sizeof(material));

  // update lighting.
  pointLight.Position = glm::vec4(std::sin(time) * 10.0f, 3.0f, 3.0f, 0.0);
  pointLight.Color = glm::vec3(1.0f, 1.0f, 1.0f);
  pointLight.Radius = 100.0f;
  vkMapMemory(mLogicalDevice, mPointLight.stagingMemory, 0, sizeof(pointLight), 0, &data);
    memcpy(data, &pointLight, sizeof(pointLight));
  vkUnmapMemory(mLogicalDevice, mPointLight.stagingMemory);
  CopyBuffer(mPointLight.stagingBuffer, mPointLight.buffer, sizeof(pointLight));

  // skybox updating.
  ubo.Model = glm::scale(glm::mat4(glm::mat3(mCamera.GetView())), glm::vec3(500.0));
  vkMapMemory(mLogicalDevice, mSkyboxUBO.stagingMemory, 0, sizeof(ubo), 0, &data);
  memcpy(data, &ubo, sizeof(ubo));
  vkUnmapMemory(mLogicalDevice, mSkyboxUBO.stagingMemory);
  CopyBuffer(mSkyboxUBO.stagingBuffer, mSkyboxUBO.buffer, sizeof(ubo));
}


void Base::MoveCamera()
{
  if (global::keyCodes[GLFW_KEY_W]) {
    mCamera.Move(Camera::FORWARD, mDt);
  }
  if (global::keyCodes[GLFW_KEY_A]) {
    mCamera.Move(Camera::LEFT, mDt);
  }
  if (global::keyCodes[GLFW_KEY_S]) {
    mCamera.Move(Camera::BACK, mDt);
  }
  if (global::keyCodes[GLFW_KEY_D]) {
    mCamera.Move(Camera::RIGHT, mDt);
  }
  if (global::keyCodes[GLFW_KEY_E]) {
    mCamera.Move(Camera::UP, mDt);
  }
  if (global::keyCodes[GLFW_KEY_Q]) {
    mCamera.Move(Camera::DOWN, mDt);
  }
}


void Base::AdjustMaterialValues()
{
  if (global::keyCodes[GLFW_KEY_M]) {
    material.metallic += 0.15f * (float )mDt;
  }
  if (global::keyCodes[GLFW_KEY_N]) {
    material.metallic -= 0.15f * (float)mDt;
  }

  if (global::keyCodes[GLFW_KEY_R]) {
    material.roughness += 0.15f * (float)mDt;
  } 
  if (global::keyCodes[GLFW_KEY_T]) {
    material.roughness -= 0.15f * (float)mDt;
  }

  if (global::keyCodes[GLFW_KEY_L]) {
    pointLight.enable = 1;
  } else if (global::keyCodes[GLFW_KEY_O]) {
    pointLight.enable = 0;
  }

  if (material.metallic < 0.1f) {
    material.metallic = 0.1f;
  }
  if (material.metallic > 1.0f) {
    material.metallic = 1.0f;
  }

  if (material.roughness > 1.0f) {
    material.roughness = 1.0f;
  }
  if (material.roughness < 0.01f) {  
    material.roughness = 0.01f;
  }
}


void Base::Run()
{
  while (!glfwWindowShouldClose(mWindow)) {
    double t = glfwGetTime();
    mDt = t - mLastTime;
    mLastTime = t;
    glfwPollEvents();
    MoveCamera();
    AdjustMaterialValues();
//    std::string str(std::to_string(m_dt) + " delta ms");
//    glfwSetWindowTitle(m_window, str.c_str());
    mCamera.Update(mDt);
    UpdateUniformBuffers();
    Draw();
    
    glfwSwapBuffers(mWindow); 
  }
  vkDeviceWaitIdle(mLogicalDevice);
  glfwTerminate();
}


void Base::Cleanup()
{
}
} // pbr