//
// Copyright (c) Mario Garcia, MIT License.
//
#ifndef __BASE_HPP
#define __BASE_HPP


#include <stdint.h>
#include "platform.hpp"
#include "camera.hpp"
#include <vulkan/vulkan.h>
#include <vector>


struct GLFWwindow;


namespace pbr {
namespace global {


extern VkInstance GetInstance();
typedef struct GLFWwindow *Window;
} // global


class Shader;


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
  Camera *GetCamera() { return &mCamera; }

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

  /// Check for SwapChain supporting details. Swap Chains are the forfront of presenting
  /// frames on the display. It acts as a queue for images that await for presentation on 
  /// screen, yet SwapChains contain formats and modes of which to present images. Details
  /// are provided inside this struct. 
  struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
  };

  /// Find our supported queue families, stored in the physical device.
  /// A default -1 is stored if no queue family exists with a graphics bit.
  QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
  SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);

  /// Creates the KHR Surface used for Vulkan to present rendering onto the window.
  /// Without it, vulkan would not know how to present onto the window display,
  /// and thus, would not be able to create Images used for rendering onto.
  void CreateSurface();

  /// Creates the Logical device from it's physical device. Sounds crazy I know,
  /// but here's the reason: The Physical Device describes the physical hardware of
  /// the GPU, but the Logical Device is pretty much the object responsible for handling
  /// of logical queues, structures, images, swapchains, etc. This is a pretty neat abstraction
  /// from the hardware features and properties, and the logical high level of the API. That way,
  /// the Graphics Programmer has an easier time understanding the computer science part, from
  /// the more specific stuff of the machine.
  void CreateLogicalDevice();

  /// Find a suitable physical device. The physical device is the GPU and it's available features,
  /// properties, extensions, and whatnot. All of this is practically vendor specific, with some 
  /// few common items found. This is practically where most of the hardware specific stuff is going
  /// to be coming from, and the programmer will likely always use the physical device to query from
  /// the actual GPU about what device they are dealing with, and what is available for use. This 
  /// pretty much separates the OS, GPU, and whatnot, from the abstract part of rendering. 
  void FindPhyiscalDevice();

  /// Setup the debug callback. Since Vulkan has you explicitly write everything to tell it what
  /// to do, we pretty much need to set up it's validation layers, as well as message structure.
  /// We also need to tell it how to relay messages back to the application when a error is caught.
  /// Tedious work, but it is a very nice thing, considering validation can be turned off to speed
  /// up performance.
  void SetDebugCallback();

  /// Create the swapchain. The Swapchain is basically a queue with images created with the 
  /// KHR surface, that is used to display onto the window. Depending on the machine, there can
  /// be multiple images that can be stored inside the swapchain, for use when displaying. 
  /// Without the swapchain, Vulkan would not know how to display the image onto the window. 
  /// This is where we deal with how an image is presented, it's color space, along with its
  /// format.
  void CreateSwapChain();

  /// Things get more interesting as we need to create ImageViews just to be able to view swapchain
  /// images. without them, there is no way of telling Vulkan how the images should be interpreted.
  /// So pretty much we are telling Vulkan how to handle these images with the swapchain, what these
  /// images are in the Images buffer, and how they need to be interpreted in this function.
  void CreateImageViews();

  /// Check if the physical device that we are inspecting, is suitable to use. This is solely
  /// based on what the application demands from the physical device, by that I mean what the
  /// programmer needs from the physical device for this application. Programmer can see the 
  /// physical device's capabilities, features, and extensions.
  bool IsDeviceSuitable(VkPhysicalDevice device);

  /// Creates the Graphics Pipeline. Two are needed, since the implementation of the
  /// sky box will have it's own pipeline.
  void CreateGraphicsPipeline();

  /// Create the renderpasses needed for our framebuffers.
  void CreateRenderPasses();
  
  /// Create our framebuffers.
  void CreateFramebuffers();

  /// Create the commandpool needed to allocate commabuffers from here.
  /// This is required to create the commands for our renderer API.
  void CreateCommandPool();
  
  /// Create the commandbuffers, it will mainly be static, but ehh.
  void CreateCommandBuffers();

  /// Create the semaphores needed for notifying the rendering API when 
  /// an image is available to present, as well as for when an image is done
  /// being drawn onto.
  void CreateSemaphores();
  
  /// Draw onto the swapchain image.
  virtual void Draw();
  void RecreateSwapchain();
  void CreateDescriptorSetLayouts();
  
  /// Simple Vertex buffer test.
  void CreateVertexBuffer();
  void CreateIndexBuffer();

  /// Create the test vertex buffer.
  void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
    VkBuffer &buffer, VkDeviceMemory &bufferMemory);
  void CreateUniformBuffers();
  void UpdateUniformBuffers();
  void CreateDescriptorPools();
  void CreateDescriptorSets();
  void CreateTextureImages();
  void SetupCamera();
  void MoveCamera();

  /// For staging buffer purposes, this will transfer the contents of the staging buffer to
  /// more high performance memory to the vertexbuffer on the gpu.
  void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

  static void OnWindowResized(global::Window window, int width, int height);
  void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, 
    VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &imageMemory);

  ///
  VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
  VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
  VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);
  uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
  VkCommandBuffer BeginSingleTimeCommands();
  void EndSingleTimeCommands(VkCommandBuffer commandbuffer);
  void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, 
    VkImageLayout newLayout);
  void CopyImage(VkImage srcImage, VkImage dstImage, uint32_t width, uint32_t height);
  void CreateImageView(VkImage image, VkFormat format, VkImageView &imageView);
  void CreateTextureImageView();
  void CreateTextureSampler();

  /// Device queue.
  struct {
    VkQueue presentation;
    VkQueue rendering;
  } mQueues;

  /// Device semaphores.
  struct {
    VkSemaphore presentation;
    VkSemaphore rendering;
  } mSemaphores;

  /// Simple test mesh.
  struct {
    VkBuffer vertexBuffer;
    VkBuffer indicesBuffer;
    VkDeviceMemory vertexMemory;
    VkDeviceMemory indicesMemory;
  } mesh;

  

  struct {
    VkImage image;
    VkImageView imageView;
    VkSampler sampler;
    VkDeviceMemory memory;
  } texture;

  /// Ubo info.
  struct {
    VkBuffer              buffer;
    VkBuffer              stagingBuffer;
    VkDeviceMemory        memory;
    VkDeviceMemory        stagingMemory;
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool      descriptorPool;
    VkDescriptorSet       descriptorSet;
  } mUbo;
  
  VkPhysicalDevice              mPhysicalDevice;
  VkDevice                      mLogicalDevice;
  VkSurfaceKHR                  mSurface; // TODO(): This needs to be global.
  VkSwapchainKHR                mSwapchain;  
  std::vector<VkImage>          mSwapchainImages;
  std::vector<VkImageView>      mSwapchainImageViews;
  std::vector<VkFramebuffer>    mSwapchainFramebuffers;
  std::vector<VkCommandBuffer>  mCommandBuffers;
  VkFormat                      mSwapchainFormat;
  VkExtent2D                    mSwapchainExtent;
  VkCommandPool                 mCommandPool;
  VkPipelineLayout              mPipelineLayout;
  VkRenderPass                  mDefaultRenderPass;
  VkPipeline                    mPbrPipeline;
  VkDebugReportCallbackEXT      mCallback;
  global::Window                mWindow;
  uint32_t                      windowWidth;
  uint32_t                      windowHeight;
  Camera                        mCamera;
  double                        mLastTime;
  double                        mDt;
};
} // pbr
#endif // __BASE_HPP