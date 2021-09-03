#pragma once

#include "cfx_window.hpp"

// std lib headers
#include <string>
#include <vector>

namespace cfx {

struct SwapChainSupportDetails {
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> presentModes;
};

struct QueueFamilyIndices {
  uint32_t graphicsFamily;
  uint32_t presentFamily;
  uint32_t transferFamily;
  bool graphicsFamilyHasValue = false;
  bool presentFamilyHasValue = false;
  bool transferFamilyHasValue = false;
  bool isComplete() { return graphicsFamilyHasValue && presentFamilyHasValue && transferFamilyHasValue; }
};

class CFXDevice {
 public:
#ifdef NDEBUG
  const bool enableValidationLayers = false;
#else
  const bool enableValidationLayers = true;
#endif

  CFXDevice(CFXWindow &window);
  ~CFXDevice();

  // Not copyable or movable
  CFXDevice(const CFXDevice &) = delete;
  void operator=(const CFXDevice &) = delete;
  CFXDevice(CFXDevice &&) = delete;
  CFXDevice &operator=(CFXDevice &&) = delete;

  VkCommandPool getCommandPool(int deviceIndex) { return commandPools[deviceIndex]; }
  VkDevice device(int deviceIndex) { return devices_[deviceIndex]; }
  std::vector<VkPhysicalDevice> getPhysicalDevices(){return physicalDevices;}
  std::vector<VkRect2D> getDeviceRects(){return deviceRects;}
  std::vector<VkSurfaceKHR> surface() { return surfaces; }
  std::vector<VkQueue> getGraphicsQueues() { return graphicsQueues; }
  std::vector<VkQueue> getPresentQueues() { return presentQueues; }
  int getDevicesinDeviceGroup(){return deviceCount; }
  VkInstance getInstance() {return instance;}
  std::string getDeviceName(int deviceIndex){
    return deviceNames[deviceIndex];
  }
  uint32_t getDeviceId(int deviceIndex){
    return deviceIds[deviceIndex];
  }
  std::vector<uint32_t> getDeviceIds(){
    return deviceIds;
  }
  std::vector<uint32_t> getDeviceMasks(){
    return deviceMasks;
  }

  std::vector<SwapChainSupportDetails> getSwapChainSupport() { return querySwapChainSupport(physicalDevices); }
  uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties,int deviceIndex);
  std::vector<QueueFamilyIndices> findPhysicalQueueFamilies() { return findQueueFamilies(physicalDevices); }
  VkFormat findSupportedFormat(
      const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

  // Buffer Helper Functions
  void createBuffer(
      VkDeviceSize size,
      VkBufferUsageFlags usage,
      VkMemoryPropertyFlags properties,
      VkBuffer &buffer,
      VkDeviceMemory &bufferMemory,int deviceIndex);
  VkCommandBuffer beginSingleTimeCommands(int deviceIndex);
  void endSingleTimeCommands(VkCommandBuffer commandBuffer,int deviceIndex);
  void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, int deviceIndex);
  void copyBufferToImage(
      VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount,int deviceIndex);

  void createImageWithInfo(
      const VkImageCreateInfo &imageInfo,
      VkMemoryPropertyFlags properties,
      VkImage &image,
      VkDeviceMemory &imageMemory, int deviceIndex);

  std::vector<VkPhysicalDeviceProperties> properties;

 private:
  void createInstance();
  void setupDebugMessenger();
  void createSurface();
  void createDeviceGroups();
  void createLogicalDevice();
  void createCommandPool(int deviceIndex);

  
  std::vector<const char *> getRequiredExtensions();
  bool checkValidationLayerSupport();
  std::vector<QueueFamilyIndices> findQueueFamilies(std::vector<VkPhysicalDevice> devices);
  void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
  void hasGflwRequiredInstanceExtensions();
  bool checkDeviceExtensionSupport(VkPhysicalDevice device);
  std::vector<SwapChainSupportDetails> querySwapChainSupport(std::vector<VkPhysicalDevice> devices);

  VkInstance instance;
  VkDebugUtilsMessengerEXT debugMessenger;
  std::vector<VkPhysicalDevice> physicalDevices;
  CFXWindow &window;
  std::vector<VkCommandPool> commandPools;
  uint32_t deviceGroupCount = 0;
  std::vector<VkPhysicalDeviceGroupProperties> physicalDeviceGroupProperties;

  std::vector<VkDevice> devices_;
  std::vector<VkSurfaceKHR> surfaces;
  uint32_t deviceCount = 0;
  
  std::vector<VkQueue> graphicsQueues;
  std::vector<VkQueue> presentQueues;
  std::vector<VkQueue> transferQueues;
  // VkQueue graphicsQueue;
  // VkQueue presentQueue;
  // VkQueue transferQueue;
  std::vector<VkRect2D> deviceRects;
  std::vector<uint32_t> deviceIds;
  std::vector<std::string> deviceNames;

  const std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};
  const std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
  std::vector<uint32_t> deviceMasks;
  std::vector<uint32_t> deviceIndices;

};

}  // namespace lve