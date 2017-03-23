//
// Copyright (c) Mario Garcia, MIT License.
//
#include "base.hpp"
#include "shader.hpp"
#include <GLFW/glfw3.h>

#include <string>
#include <set>
#include <vector>
#include <assert.h>
#include <algorithm>
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
  : m_swapchain(VK_NULL_HANDLE)
{
  glfwInit();
}


Base::~Base()
{
  CloseWindow();
  Cleanup();
  vkDestroySemaphore(m_logicalDev, m_semaphores.presentation, nullptr);
  vkDestroySemaphore(m_logicalDev, m_semaphores.rendering, nullptr);
  // Destroy swapchain image views
  for (size_t i = 0; i < m_swapchainImageViews.size(); ++i) {
    vkDestroyImageView(m_logicalDev, m_swapchainImageViews[i], nullptr);
    vkDestroyFramebuffer(m_logicalDev, m_swapchainFramebuffers[i], nullptr);
  }

  vkFreeCommandBuffers(m_logicalDev, m_commandPool, 
    static_cast<uint32_t>(m_commandbuffers.size()), m_commandbuffers.data());

  vkDestroyCommandPool(m_logicalDev, m_commandPool, nullptr);
  vkDestroyPipeline(m_logicalDev, m_pbrPipeline, nullptr);
  vkDestroyRenderPass(m_logicalDev, m_defaultRenderPass, nullptr);
  vkDestroyPipelineLayout(m_logicalDev, m_pipelineLayout, nullptr);
  vkDestroySwapchainKHR(m_logicalDev, m_swapchain, nullptr);
  vkDestroySurfaceKHR(global::GetInstance(), m_surface, nullptr);
  vkDestroyDevice(m_logicalDev, nullptr);
  DestroyDebugReportCallbackEXT(global::GetInstance(), m_callback, nullptr);
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
  m_window = glfwCreateWindow(width, height, "PBR Test Vulkan", nullptr, nullptr);
  glfwMakeContextCurrent(m_window);

  // Set the reference to the object that has ownership of this window.
  glfwSetWindowUserPointer(m_window, this);

  glfwSetKeyCallback(m_window, global::KeyCallback);
  glfwSetWindowSizeCallback(m_window, Base::OnWindowResized);
#if defined(_WIN32)
  //HWND console = GetConsoleWindow();
  //ShowWindow(console, SW_HIDE);
#endif
}


Base::SwapChainSupportDetails Base::QuerySwapChainSupport(VkPhysicalDevice device)
{
  SwapChainSupportDetails details;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &details.capabilities);
  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, nullptr);
  if (formatCount > 0) {
    details.formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, 
      &formatCount, details.formats.data());
  }
  uint32_t presentCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentCount, nullptr);
  if (presentCount > 0) {
    details.presentModes.resize(presentCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, 
      &presentCount, details.presentModes.data());
  }

  return details;
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
      m_physicalDev = device;
      break;
    }
  }
  BASE_ASSERT(m_physicalDev != VK_NULL_HANDLE && "Base member m_phyDev is still NULL!!"); 
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
    glfwGetWindowSize(m_window, &width, &height);
    VkExtent2D actualExtent = { width, height };
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
    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &presentSupport);
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
  QueueFamilyIndices indices = FindQueueFamilies(m_physicalDev);
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
  VkResult result = vkCreateDevice(m_physicalDev, &deviceCreateInfo, nullptr, &m_logicalDev);
  BASE_ASSERT(result == VK_SUCCESS  && "Failed to create a logical device...");

  // Get the device queue.
  vkGetDeviceQueue(m_logicalDev, indices.graphicsFamily, 0, &m_queue.rendering);
  vkGetDeviceQueue(m_logicalDev, indices.presentFamily, 0, &m_queue.presentation);
}


void Base::CreateSurface()
{
  VkResult result = glfwCreateWindowSurface(global::GetInstance(),
    m_window, nullptr, &m_surface);
  BASE_ASSERT(result == VK_SUCCESS && "Failed to Create a KHR surface!");
}


void Base::CreateSwapChain()
{
  SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(m_physicalDev);
  VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
  VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
  VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

  uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
  if (swapChainSupport.capabilities.maxImageCount > 0 && 
      imageCount > swapChainSupport.capabilities.maxImageCount) {
    imageCount = swapChainSupport.capabilities.maxImageCount;
  }
  VkSwapchainKHR old_swapchain = m_swapchain;
  VkSwapchainKHR new_swapchain;
  VkSwapchainCreateInfoKHR swapChainCreateInfo = { };
  swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  swapChainCreateInfo.surface = m_surface;
  swapChainCreateInfo.minImageCount = imageCount;
  swapChainCreateInfo.imageFormat = surfaceFormat.format;
  swapChainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
  swapChainCreateInfo.imageExtent = extent;
  swapChainCreateInfo.imageArrayLayers = 1;
  // No offscreen rendering, unless needed. 
  swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; 

  QueueFamilyIndices indices = FindQueueFamilies(m_physicalDev);
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
 
  VkResult result = vkCreateSwapchainKHR(m_logicalDev, &swapChainCreateInfo, nullptr, &new_swapchain);
  BASE_ASSERT(result == VK_SUCCESS && "Swapchain failed to create!"); 
  m_swapchain = new_swapchain;
  vkGetSwapchainImagesKHR(m_logicalDev, m_swapchain, &imageCount, nullptr);
  m_swapchainImages.resize(imageCount);
  vkGetSwapchainImagesKHR(m_logicalDev, m_swapchain, &imageCount, m_swapchainImages.data());

  // Keep track of swapchain format at extent.
  m_swapchainFormat = surfaceFormat.format;
  m_swapchainExtent = extent;

  // If the old swapchain is a null handle, ignore having to delete it.
  if (old_swapchain != VK_NULL_HANDLE) {
    vkDestroySwapchainKHR(m_logicalDev, old_swapchain, nullptr);
  }
}


void Base::CreateImageViews()
{
  m_swapchainImageViews.resize(m_swapchainImages.size());
  for (uint32_t i = 0; i < m_swapchainImages.size(); ++i) {
    VkImageViewCreateInfo imageViewCreateInfo = { };
    imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCreateInfo.image = m_swapchainImages[i];
    imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewCreateInfo.format = m_swapchainFormat;
    imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    imageViewCreateInfo.subresourceRange.layerCount = 1;
    imageViewCreateInfo.subresourceRange.levelCount = 1;
    VkResult result = vkCreateImageView(m_logicalDev, &imageViewCreateInfo, nullptr, &m_swapchainImageViews[i]);
    BASE_ASSERT(result == VK_SUCCESS && "Swapchain ImageView failed to create!");
  }
}


void Base::CreateGraphicsPipeline()
{
  VkShaderModule vert = ShaderModule::GenerateShaderModule(m_logicalDev, 
    ShaderModule::ssVertShader, "../../pbr-study/shaders/test.vert");
  VkShaderModule frag = ShaderModule::GenerateShaderModule(m_logicalDev,
    ShaderModule::ssFragShader, "../../pbr-study/shaders/test.frag");

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

  VkPipelineVertexInputStateCreateInfo vertexInputStateinfo = { };
  vertexInputStateinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputStateinfo.pVertexAttributeDescriptions = nullptr;
  vertexInputStateinfo.vertexAttributeDescriptionCount = 0;
  vertexInputStateinfo.pVertexBindingDescriptions = nullptr;
  vertexInputStateinfo.vertexBindingDescriptionCount = 0;

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
  viewport.width = static_cast<float>(m_swapchainExtent.width);
  viewport.height = static_cast<float>(m_swapchainExtent.height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;  
  
  VkRect2D scissor = { };
  scissor.offset.x = scissor.offset.y = 0;
  scissor.extent = m_swapchainExtent;

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
  rasterStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
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
  pipelineLayoutCreatInfo.setLayoutCount = 0;
  pipelineLayoutCreatInfo.pSetLayouts = nullptr;
  pipelineLayoutCreatInfo.pushConstantRangeCount = 0;
  pipelineLayoutCreatInfo.pPushConstantRanges = nullptr;

  VkResult result = vkCreatePipelineLayout(m_logicalDev, 
    &pipelineLayoutCreatInfo, nullptr, &m_pipelineLayout);
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
  gPipelineCreateInfo.layout = m_pipelineLayout;
  gPipelineCreateInfo.renderPass = m_defaultRenderPass;
  // For deriving from a base pipeline (parent). This is a single pipeline, so
  // we aren't deriving from any other pipeline.
  gPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
  gPipelineCreateInfo.basePipelineIndex = -1;

  result = vkCreateGraphicsPipelines(m_logicalDev, VK_NULL_HANDLE, 1, 
    &gPipelineCreateInfo, nullptr, &m_pbrPipeline);
  BASE_ASSERT(result == VK_SUCCESS && "Failed to create PBR graphics pipeline!");

  vkDestroyShaderModule(m_logicalDev, vert, nullptr);
  vkDestroyShaderModule(m_logicalDev, frag, nullptr);
}


void Base::CreateRenderPasses()
{
  // This is the actual attachment to be bound to 
  // the renderpass.
  VkAttachmentDescription colorAttachment = { };
  colorAttachment.format = m_swapchainFormat;
  colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  // layout (location = 0) reference. Can be used in multiple renderpasses.
  VkAttachmentReference attachmentRef = { };
  attachmentRef.attachment = 0; // attachment location
  attachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  // This subpass describes attachment references, as well as it's binding 
  // point (which is for graphics pipelines, whereas it can also bind to 
  // compute pipelines if explicitly told so) to be used for multiple renderpasses.
  VkSubpassDescription subpass = { };
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &attachmentRef;

  VkSubpassDependency subpassDependency = { };
  subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  subpassDependency.dstSubpass = 0;
  subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  subpassDependency.srcAccessMask = 0;
  subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | 
    VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  

  VkRenderPassCreateInfo renderpassCreateInfo = { };
  renderpassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderpassCreateInfo.attachmentCount = 1;
  renderpassCreateInfo.pAttachments = &colorAttachment;
  renderpassCreateInfo.subpassCount = 1;
  renderpassCreateInfo.pSubpasses = &subpass;
  renderpassCreateInfo.dependencyCount = 1;
  renderpassCreateInfo.pDependencies = &subpassDependency;
  
  VkResult result = vkCreateRenderPass(m_logicalDev, &renderpassCreateInfo, nullptr, &m_defaultRenderPass);
  BASE_ASSERT(result == VK_SUCCESS);
}


void Base::CreateFramebuffers()
{
  m_swapchainFramebuffers.resize(m_swapchainImageViews.size());
  for (size_t i = 0; i < m_swapchainImageViews.size(); ++i) {
    VkImageView attachments[] {
      m_swapchainImageViews[i]
    };
    VkFramebufferCreateInfo framebufferCreateInfo = { };
    framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferCreateInfo.renderPass = m_defaultRenderPass;
    framebufferCreateInfo.attachmentCount = 1;
    framebufferCreateInfo.pAttachments = attachments;
    framebufferCreateInfo.width = m_swapchainExtent.width;
    framebufferCreateInfo.height = m_swapchainExtent.height;
    framebufferCreateInfo.layers = 1;
    VkResult result = vkCreateFramebuffer(m_logicalDev, &framebufferCreateInfo, 
      nullptr, &m_swapchainFramebuffers[i]);
    BASE_ASSERT(result == VK_SUCCESS && "A Swapchain Framebuffer failed to create!");
  }
}


void Base::CreateCommandPool()
{
  QueueFamilyIndices indices = FindQueueFamilies(m_physicalDev);
  VkCommandPoolCreateInfo commandPoolCreateInfo = { };
  commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  commandPoolCreateInfo.queueFamilyIndex = indices.graphicsFamily;
  commandPoolCreateInfo.flags = 0;
  VkResult result = vkCreateCommandPool(m_logicalDev, &commandPoolCreateInfo, nullptr, &m_commandPool);
  BASE_ASSERT(result == VK_SUCCESS && "Failed to create a command pool!");
}


void Base::CreateCommandBuffers()
{
  m_commandbuffers.resize(m_swapchainFramebuffers.size());

  {
    VkCommandBufferAllocateInfo cmdAllocInfo = { };
    cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdAllocInfo.commandPool = m_commandPool;
    cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdAllocInfo.commandBufferCount = static_cast<uint32_t>(m_commandbuffers.size());
    VkResult result = vkAllocateCommandBuffers(m_logicalDev, &cmdAllocInfo, m_commandbuffers.data());
    BASE_ASSERT(result == VK_SUCCESS && "Failed to allocated commandbuffers!");
  }

  for (size_t i = 0; i < m_commandbuffers.size(); ++i) {
    VkCommandBufferBeginInfo cmdBeginInfo = { };
    cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBeginInfo.pInheritanceInfo = nullptr;
    cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    cmdBeginInfo.pNext = nullptr;
    vkBeginCommandBuffer(m_commandbuffers[i], &cmdBeginInfo);    
    
    VkRenderPassBeginInfo renderpassBegin = { };
    renderpassBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderpassBegin.renderPass = m_defaultRenderPass;
    renderpassBegin.framebuffer = m_swapchainFramebuffers[i];
    renderpassBegin.renderArea.offset = { 0, 0 };
    renderpassBegin.renderArea.extent = m_swapchainExtent;
    VkClearValue clearcolor = { 0.0, 0.0f, 0.0f, 1.0f };
    renderpassBegin.clearValueCount = 1;
    renderpassBegin.pClearValues = &clearcolor;
    vkCmdBeginRenderPass(m_commandbuffers[i], &renderpassBegin, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(m_commandbuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pbrPipeline);
    // hardcoded values.
    vkCmdDraw(m_commandbuffers[i], 3, 1, 0, 0);
    vkCmdEndRenderPass(m_commandbuffers[i]);
    VkResult result = vkEndCommandBuffer(m_commandbuffers[i]);
    BASE_ASSERT(result == VK_SUCCESS && "A CommandBuffer failed recording!");
  }
}


void Base::CreateSemaphores()
{
  VkSemaphoreCreateInfo semaphoreCreateInfo = { };
  semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  VkResult result = vkCreateSemaphore(m_logicalDev, &semaphoreCreateInfo, 
    nullptr, &m_semaphores.presentation);
  BASE_ASSERT(result == VK_SUCCESS && "Failed to create presentation semaphore!");
  result = vkCreateSemaphore(m_logicalDev, &semaphoreCreateInfo,
    nullptr, &m_semaphores.rendering);
  BASE_ASSERT(result == VK_SUCCESS && "Failed to create rendering semaphore!");
  
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
  CreateGraphicsPipeline();
  CreateFramebuffers();
  CreateCommandPool();
  CreateCommandBuffers();
  CreateSemaphores();
}


void Base::Draw()
{
  uint32_t image_index;
  vkAcquireNextImageKHR(m_logicalDev, m_swapchain, (std::numeric_limits<uint64_t>::max)(),
    m_semaphores.presentation, VK_NULL_HANDLE, &image_index);
  VkSubmitInfo submitInfo = { };
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  VkSemaphore wait_semaphores[] = { m_semaphores.presentation };
  VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = wait_semaphores;
  submitInfo.pWaitDstStageMask = wait_stages;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &m_commandbuffers[image_index];
  
  VkSemaphore signal_semaphores[] = { m_semaphores.rendering };
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = signal_semaphores;
  VkResult result = vkQueueSubmit(m_queue.rendering, 1, &submitInfo, VK_NULL_HANDLE); 
  BASE_ASSERT(result == VK_SUCCESS && "Failed to submit commandbuffer to rendering queue.");
  
  VkPresentInfoKHR presentInfo = { };
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = signal_semaphores;
  VkSwapchainKHR swapchains[] = { m_swapchain };
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = swapchains;
  presentInfo.pImageIndices = &image_index;
  presentInfo.pResults = nullptr;
  vkQueuePresentKHR(m_queue.presentation, &presentInfo);
}


void Base::RecreateSwapchain()
{
  vkDeviceWaitIdle(m_logicalDev);

  for (size_t i = 0; i < m_swapchainImageViews.size(); ++i) {
    vkDestroyImageView(m_logicalDev, m_swapchainImageViews[i], nullptr);
    vkDestroyFramebuffer(m_logicalDev, m_swapchainFramebuffers[i], nullptr);
  }

  vkFreeCommandBuffers(m_logicalDev, m_commandPool,
    static_cast<uint32_t>(m_commandbuffers.size()), m_commandbuffers.data());

  vkDestroyPipeline(m_logicalDev, m_pbrPipeline, nullptr);
  vkDestroyRenderPass(m_logicalDev, m_defaultRenderPass, nullptr);
  vkDestroyPipelineLayout(m_logicalDev, m_pipelineLayout, nullptr);

  CreateSwapChain();
  CreateImageViews();
  CreateRenderPasses();
  CreateGraphicsPipeline();
  CreateFramebuffers();
  CreateCommandBuffers();
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
    Draw();
    
    glfwSwapBuffers(m_window); 
  }
  vkDeviceWaitIdle(m_logicalDev);
  glfwTerminate();
}


void Base::Cleanup()
{
}
} // pbr