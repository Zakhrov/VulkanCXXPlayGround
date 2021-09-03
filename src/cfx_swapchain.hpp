#pragma once

#include "cfx_device.hpp"

#include<vulkan/vulkan.h>
#include<string>
#include<vector>
#include <memory>


namespace cfx{
    class CFXSwapChain{
        public:
  static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

  CFXSwapChain(CFXDevice &deviceRef, VkExtent2D windowExtent);
  CFXSwapChain(CFXDevice &deviceRef, VkExtent2D windowExtent,std::shared_ptr<CFXSwapChain> previous);
  ~CFXSwapChain();

  CFXSwapChain(const CFXSwapChain &) = delete;
  void operator=(const CFXSwapChain &) = delete;

  VkFramebuffer getFrameBuffer(int deviceIndex,int index) { return swapChainFramebuffers[deviceIndex][index]; }
  VkRenderPass getRenderPass(int deviceIndex) { return renderPasses[deviceIndex]; }
  std::vector<VkRenderPass> getRenderPasses() { return renderPasses; }
  VkImageView getImageView(int deviceIndex,int index) { return swapChainImageViews[deviceIndex][index]; }
  size_t imageCount(int deviceIndex) { return swapChainImages[deviceIndex].size(); }
  VkFormat getSwapChainImageFormat() { return swapChainImageFormat; }
  VkExtent2D getSwapChainExtent() { return swapChainExtent; }
  uint32_t width() { return swapChainExtent.width; }
  uint32_t height() { return swapChainExtent.height; }

  float extentAspectRatio() {
    return static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height);
  }
  VkFormat findDepthFormat();

  VkResult acquireNextImage(uint32_t *imageIndex,uint32_t deviceIndex);
  VkResult submitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex,uint32_t deviceIndex);
  bool compareSwapFormats(const CFXSwapChain& cfxSwapChain) const {
      return cfxSwapChain.swapChainDepthFormat == swapChainDepthFormat && cfxSwapChain.swapChainImageFormat == swapChainImageFormat;
  }

 private:
  void createSwapChain(uint32_t deviceIndex);
  void init();
  void createImageViews(uint32_t deviceIndex);
  void createDepthResources(uint32_t deviceIndex);
  void createRenderPass(uint32_t deviceIndex);
  void createFramebuffers(uint32_t deviceIndex);
  void createSyncObjects(uint32_t deviceIndex);

  // Helper functions
  VkSurfaceFormatKHR chooseSwapSurfaceFormat(
      const std::vector<VkSurfaceFormatKHR> &availableFormats);
  VkPresentModeKHR chooseSwapPresentMode(
      const std::vector<VkPresentModeKHR> &availablePresentModes);
  VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

  VkFormat swapChainImageFormat;
  VkFormat swapChainDepthFormat;
  VkExtent2D swapChainExtent;

  std::vector<std::vector<VkFramebuffer>> swapChainFramebuffers;
  std::vector<VkRenderPass> renderPasses{};

  std::vector<std::vector<VkImage>> depthImages;
  std::vector<std::vector<VkDeviceMemory>> depthImageMemorys;
  std::vector<std::vector<VkImageView>> depthImageViews;
  std::vector<std::vector<VkImage>> swapChainImages;
  std::vector<std::vector<VkImageView>> swapChainImageViews;

  CFXDevice &device;
  VkExtent2D windowExtent;

  std::vector<VkSwapchainKHR> swapChains;
  std::shared_ptr<CFXSwapChain> oldSwapChain;


  std::vector<std::vector<VkSemaphore>> imageAvailableSemaphores;
  std::vector<std::vector<VkSemaphore>> renderFinishedSemaphores;
  std::vector<std::vector<VkFence>> inFlightFences;
  std::vector<std::vector<VkFence>> imagesInFlight;
  size_t currentFrame = 0;
  std::vector<uint32_t> deviceMasks = {1,2};
    };
}