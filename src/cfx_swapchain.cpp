#include "cfx_swapchain.hpp"

#include <array>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <set>
#include <stdexcept>

namespace cfx
{
  CFXSwapChain::CFXSwapChain(CFXDevice &deviceRef, VkExtent2D extent)
      : device{deviceRef}, windowExtent{extent}
  {
    init();
  }
  CFXSwapChain::CFXSwapChain(CFXDevice &deviceRef, VkExtent2D extent, std::shared_ptr<CFXSwapChain> previous)
      : device{deviceRef}, windowExtent{extent}, oldSwapChain{previous}
  {

    init();
    oldSwapChain = nullptr;
  }
  void CFXSwapChain::init()
  {
    swapChains.resize(device.getDevicesinDeviceGroup());
    swapChainImages.resize(device.getDevicesinDeviceGroup());
    swapChainImageViews.resize(device.getDevicesinDeviceGroup());
    swapChainFramebuffers.resize(device.getDevicesinDeviceGroup());
    depthImages.resize(device.getDevicesinDeviceGroup());
    depthImageMemorys.resize(device.getDevicesinDeviceGroup());
    depthImageViews.resize(device.getDevicesinDeviceGroup());
    imageAvailableSemaphores.resize(device.getDevicesinDeviceGroup());
    renderFinishedSemaphores.resize(device.getDevicesinDeviceGroup());
    inFlightFences.resize(device.getDevicesinDeviceGroup());
    imagesInFlight.resize(device.getDevicesinDeviceGroup());
    swapChainImageFormat.resize(device.getDevicesinDeviceGroup());
    swapChainDepthFormat.resize(device.getDevicesinDeviceGroup());
    swapChainExtent.resize(device.getDevicesinDeviceGroup());
    renderPasses.resize(device.getDevicesinDeviceGroup());

    for (int deviceIndex = 0; deviceIndex < device.getDevicesinDeviceGroup(); deviceIndex++)
    {
      createSwapChain(deviceIndex);
      createImageViews(deviceIndex);
      createDepthResources(deviceIndex);
      createRenderPass(deviceIndex);
      createFramebuffers(deviceIndex);
      createSyncObjects(deviceIndex);
    }
  }

  CFXSwapChain::~CFXSwapChain()
  {
    for (int deviceIndex = 0; deviceIndex < device.getDevicesinDeviceGroup(); deviceIndex++)
    {

      for (int i = 0; i < swapChainImageViews[deviceIndex].size(); i++)
      {
        vkDestroyImageView(device.device(deviceIndex), swapChainImageViews[deviceIndex][i], nullptr);
      }

      // for(int i=0; i < swapChainImages[deviceIndex].size();i++){

      //     vkDestroyImage(device.device(deviceIndex), swapChainImages[deviceIndex][i], nullptr);
      // }
      vkDestroySwapchainKHR(device.device(deviceIndex), swapChains[deviceIndex], nullptr);

      for (int i = 0; i < depthImages[deviceIndex].size(); i++)
      {
        vkDestroyImageView(device.device(deviceIndex), depthImageViews[deviceIndex][i], nullptr);
        vkDestroyImage(device.device(deviceIndex), depthImages[deviceIndex][i], nullptr);
        vkFreeMemory(device.device(deviceIndex), depthImageMemorys[deviceIndex][i], nullptr);
      }

      for (auto frameBufferArray : swapChainFramebuffers[deviceIndex])
      {
        vkDestroyFramebuffer(device.device(deviceIndex), frameBufferArray, nullptr);
      }

      // cleanup synchronization objects
      vkDestroyRenderPass(device.device(deviceIndex), renderPasses[deviceIndex], nullptr);
      destroySyncObjects(deviceIndex);
    }
  }

  VkResult CFXSwapChain::acquireNextImage(uint32_t *imageIndex, uint32_t deviceIndex)
  {

    vkWaitForFences(
        device.device(deviceIndex),
        1,
        &inFlightFences[deviceIndex][currentFrame],
        VK_TRUE,
        std::numeric_limits<uint64_t>::max());
    VkAcquireNextImageInfoKHR nextImageInfo{};
    nextImageInfo.sType = VK_STRUCTURE_TYPE_ACQUIRE_NEXT_IMAGE_INFO_KHR;
    nextImageInfo.swapchain = swapChains[deviceIndex];
    nextImageInfo.timeout = std::numeric_limits<uint64_t>::max();
    nextImageInfo.semaphore = imageAvailableSemaphores[deviceIndex][currentFrame];
    nextImageInfo.fence = VK_NULL_HANDLE;
    // nextImageInfo.deviceMask = deviceMasks[deviceIndex];
    // return vkAcquireNextImage2KHR(device.device(deviceIndex),&nextImageInfo,imageIndex);
    VkResult result = vkAcquireNextImageKHR(
        device.device(deviceIndex),
        swapChains[deviceIndex],
        std::numeric_limits<uint64_t>::max(),
        imageAvailableSemaphores[deviceIndex][currentFrame], // must be a not signaled semaphore
        VK_NULL_HANDLE,
        imageIndex);
    // std::cout << " NEXT IMG ACQUIRED " << std::endl;
    return result;
  }
  void CFXSwapChain::destroySyncObjects(int deviceIndex)
  {
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
      vkDestroySemaphore(device.device(deviceIndex), renderFinishedSemaphores[deviceIndex][i], nullptr);
      vkDestroySemaphore(device.device(deviceIndex), imageAvailableSemaphores[deviceIndex][i], nullptr);
      vkDestroyFence(device.device(deviceIndex), inFlightFences[deviceIndex][i], nullptr);
    }
  }

  VkResult CFXSwapChain::submitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex, uint32_t deviceIndex)
  {

    // std::cout << "INSIDE SUBMIT_COMMAND_BUFFER FOR DEVICE " << device.getDeviceName(deviceIndex) << std::endl;
    // std::cout << device.getDeviceName(deviceIndex) << " CURRENT FRAME " << currentFrame << std::endl;
    // std::cout << device.getDeviceName(deviceIndex) << " IMAGE INDEX " << imageIndex << std::endl;
    // std::cout << device.getDeviceName(deviceIndex) << " IMAGES IN FLIGHT " << imagesInFlight[deviceIndex].size() << std::endl;
    // std::cout << device.getDeviceName(deviceIndex) << " IN FLIGHT FENCES " << inFlightFences[deviceIndex].size() << std::endl;

    imagesInFlight[deviceIndex][*imageIndex] = inFlightFences[deviceIndex][currentFrame];

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    std::vector<VkSemaphore> waitSemaphores = {imageAvailableSemaphores[deviceIndex][currentFrame]};
    std::vector<uint32_t> waitSemaphoreIndices = {0};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = waitSemaphores.size();
    submitInfo.pWaitSemaphores = waitSemaphores.data();
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = buffers;

    std::vector<VkSemaphore> signalSemaphores = {renderFinishedSemaphores[deviceIndex][currentFrame]};
    std::vector<uint32_t> signalSemaphoreIndices = {0};
    submitInfo.signalSemaphoreCount = signalSemaphores.size();
    submitInfo.pSignalSemaphores = signalSemaphores.data();

    VkResult result = vkResetFences(device.device(deviceIndex), 1, &inFlightFences[deviceIndex][currentFrame]);
    if (result == VK_SUCCESS)
    {

      if (vkQueueSubmit(device.getGraphicsQueues(deviceIndex), 1, &submitInfo, inFlightFences[deviceIndex][currentFrame]) !=
          VK_SUCCESS)
      {
        throw std::runtime_error("failed to submit draw command buffer!");
      }

      VkPresentInfoKHR presentInfo = {};
      presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

      presentInfo.waitSemaphoreCount = signalSemaphores.size();
      presentInfo.pWaitSemaphores = signalSemaphores.data();

      presentInfo.swapchainCount = 1;
      presentInfo.pSwapchains = &swapChains[deviceIndex];

      presentInfo.pImageIndices = imageIndex;

      VkResult result = vkQueuePresentKHR(device.getPresentQueues(deviceIndex), &presentInfo);
      // std::cout<< "CURRENT FRAME >>>>>> "<< currentFrame << std::endl;
      currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

      return result;
    }
    else
    {
      throw std::runtime_error("failed to reset Fence!");
      return result;
    }
  }

  void CFXSwapChain::createSwapChain(int deviceIndex)
  {

    SwapChainSupportDetails swapChainSupport = device.getSwapChainSupport(deviceIndex);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes, deviceIndex);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    
    if (swapChainSupport.capabilities.maxImageCount > 0 &&
        imageCount > swapChainSupport.capabilities.maxImageCount)
    {
      imageCount = swapChainSupport.capabilities.maxImageCount;
    }
    

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = device.surface();

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = device.findPhysicalQueueFamilies(deviceIndex);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily, indices.presentFamily};

    if (indices.graphicsFamily != indices.presentFamily)
    {
      createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
      createInfo.queueFamilyIndexCount = 2;
      createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
      createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
      createInfo.queueFamilyIndexCount = 0;     // Optional
      createInfo.pQueueFamilyIndices = nullptr; // Optional
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(device.device(deviceIndex), &createInfo, nullptr, &swapChains[deviceIndex]) != VK_SUCCESS)
    {
      throw std::runtime_error("failed to create swap chain!");
    }
    // std::cout<< "CREATE SWAP CHAIN " << std::endl;

    // we only specified a minimum number of images in the swap chain, so the implementation is
    // allowed to create a swap chain with more. That's why we'll first query the final number of
    // images with vkGetSwapchainImagesKHR, then resize the container and finally call it again to
    // retrieve the handles.
    if (vkGetSwapchainImagesKHR(device.device(deviceIndex), swapChains[deviceIndex], &imageCount, nullptr) != VK_SUCCESS)
    {
      throw std::runtime_error("failed to get swap chain images!");
    }
    swapChainImages[deviceIndex].resize(imageCount);
    
    if (vkGetSwapchainImagesKHR(device.device(deviceIndex), swapChains[deviceIndex], &imageCount, swapChainImages[deviceIndex].data()) != VK_SUCCESS)
    {
      throw std::runtime_error("failed to get swap chain images!");
    }
    swapChainImageFormat[deviceIndex] = surfaceFormat.format;
    swapChainExtent[deviceIndex] = extent;
  }

  void CFXSwapChain::createImageViews(int deviceIndex)
  {
    
    swapChainImageViews[deviceIndex].resize(swapChainImages[deviceIndex].size());
    for (size_t i = 0; i < swapChainImages[deviceIndex].size(); i++)
    {
      VkImageViewCreateInfo viewInfo{};
      viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
      viewInfo.image = swapChainImages[deviceIndex][i];
      viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
      viewInfo.format = swapChainImageFormat[deviceIndex];
      viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      viewInfo.subresourceRange.baseMipLevel = 0;
      viewInfo.subresourceRange.levelCount = 1;
      viewInfo.subresourceRange.baseArrayLayer = 0;
      viewInfo.subresourceRange.layerCount = 1;

      if (vkCreateImageView(device.device(deviceIndex), &viewInfo, nullptr, &swapChainImageViews[deviceIndex][i]) !=
          VK_SUCCESS)
      {
        throw std::runtime_error("failed to create texture image view!");
      }
    }
    // std::cout<< "CREATE IMAGE VIEWS END " << std::endl;
  }

  void CFXSwapChain::createRenderPass(int deviceIndex)
  {
    // std::cout << "CREATE RENDER PASS ON " << device.getDeviceName(deviceIndex) << std::endl;
    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = findDepthFormat();
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = getSwapChainImageFormat(deviceIndex);
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.srcAccessMask = 0;
    dependency.srcStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstSubpass = 0;
    dependency.dstStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask =
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(device.device(deviceIndex), &renderPassInfo, nullptr, &renderPasses[deviceIndex]) != VK_SUCCESS)
    {
      throw std::runtime_error("failed to create render pass!");
    }
    // std::cout << "CREATE RENDER PASS END " <<std::endl;
  }

  void CFXSwapChain::createFramebuffers(int deviceIndex)
  {
    // std::cout << "CREATE FRAME BUFFER " <<std::endl;
    // std::cout << "CREATE FRAME BUFFER SWAP CHAIN SIZE 1 " << swapChainFramebuffers[deviceIndex].size() <<std::endl;
    // std::cout << "CREATE FRAME BUFFER IMAGE COUNT " << imageCount(deviceIndex) <<std::endl;

    swapChainFramebuffers[deviceIndex].resize(imageCount(deviceIndex));
    // std::cout << "CREATE FRAME BUFFER SWAP CHAIN SIZE 2 " << swapChainFramebuffers[deviceIndex].size() <<std::endl;
    for (size_t i = 0; i < imageCount(deviceIndex); i++)
    {
      // std::cout << "CREATE FRAME BUFFER ATTACHMENTS " << i  <<std::endl;
      // std::cout << "SWAP CHAIN IMAGE VIEW SIZE " << swapChainImageViews[deviceIndex].size() << std::endl;
      // std::cout << "DEPTH IMAGE VIEW SIZE " << depthImageViews[deviceIndex].size() << std::endl;
      if (swapChainImageViews[deviceIndex][i] == nullptr || depthImageViews[deviceIndex][i] == nullptr)
      {
        std::cout << "BREAKING AT " << i << std::endl;
        break;
      }
      std::array<VkImageView, 2> attachments = {swapChainImageViews[deviceIndex][i], depthImageViews[deviceIndex][i]};

      VkExtent2D swapChainExtent = getSwapChainExtent(deviceIndex);
      VkFramebufferCreateInfo framebufferInfo = {};
      framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
      framebufferInfo.renderPass = renderPasses[deviceIndex];
      framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
      framebufferInfo.pAttachments = attachments.data();
      framebufferInfo.width = swapChainExtent.width;
      framebufferInfo.height = swapChainExtent.height;
      framebufferInfo.layers = 1;
      // std::cout << "CREATE FRAME BUFFER FUNCTION CALL"  <<std::endl;

      if (vkCreateFramebuffer(
              device.device(deviceIndex),
              &framebufferInfo,
              nullptr,
              &swapChainFramebuffers[deviceIndex][i]) != VK_SUCCESS)
      {
        throw std::runtime_error("failed to create framebuffer!");
      }
    }
    // std::cout << "CREATE FRAME BUFFER END " <<std::endl;
  }

  void CFXSwapChain::createDepthResources(int deviceIndex)
  {
    // std::cout << "CREATE DEPTH RESOURCES "  <<std::endl;
    VkFormat depthFormat = findDepthFormat();
    swapChainDepthFormat[deviceIndex] = depthFormat;
    VkExtent2D swapChainExtent = getSwapChainExtent(deviceIndex);

    depthImages[deviceIndex].resize(imageCount(deviceIndex));
    depthImageMemorys[deviceIndex].resize(imageCount(deviceIndex));
    depthImageViews[deviceIndex].resize(imageCount(deviceIndex));

    for (int i = 0; i < depthImages[deviceIndex].size(); i++)
    {
      VkImageCreateInfo imageInfo{};
      imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
      imageInfo.imageType = VK_IMAGE_TYPE_2D;
      imageInfo.extent.width = swapChainExtent.width;
      imageInfo.extent.height = swapChainExtent.height;
      imageInfo.extent.depth = 1;
      imageInfo.mipLevels = 1;
      imageInfo.arrayLayers = 1;
      imageInfo.format = depthFormat;
      imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
      imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
      imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
      imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
      imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
      imageInfo.flags = VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT;

      device.createImageWithInfo(
          imageInfo,
          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
          depthImages[deviceIndex][i],
          depthImageMemorys[deviceIndex][i], deviceIndex);

      VkImageViewCreateInfo viewInfo{};
      viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
      viewInfo.image = depthImages[deviceIndex][i];
      viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
      viewInfo.format = depthFormat;
      viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
      viewInfo.subresourceRange.baseMipLevel = 0;
      viewInfo.subresourceRange.levelCount = 1;
      viewInfo.subresourceRange.baseArrayLayer = 0;
      viewInfo.subresourceRange.layerCount = 1;

      if (vkCreateImageView(device.device(deviceIndex), &viewInfo, nullptr, &depthImageViews[deviceIndex][i]) != VK_SUCCESS)
      {
        throw std::runtime_error("failed to create texture image view!");
      }
    }
    // std::cout << "CREATE DEPTH RESOURCES END"   <<std::endl;
  }

  void CFXSwapChain::createSyncObjects(int deviceIndex)
  {
    // std::cout << "CREATE SYNC OBJECTS "   <<std::endl;
    imageAvailableSemaphores[deviceIndex].resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores[deviceIndex].resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences[deviceIndex].resize(MAX_FRAMES_IN_FLIGHT);
    imagesInFlight[deviceIndex].resize(imageCount(deviceIndex), VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
      if (vkCreateSemaphore(device.device(deviceIndex), &semaphoreInfo, nullptr, &imageAvailableSemaphores[deviceIndex][i]) !=
              VK_SUCCESS ||
          vkCreateSemaphore(device.device(deviceIndex), &semaphoreInfo, nullptr, &renderFinishedSemaphores[deviceIndex][i]) !=
              VK_SUCCESS ||
          vkCreateFence(device.device(deviceIndex), &fenceInfo, nullptr, &inFlightFences[deviceIndex][i]) != VK_SUCCESS)
      {
        throw std::runtime_error("failed to create synchronization objects for a frame!");
      }
    }

    // std::cout << "CREATE SYNC OBJECTS END "  <<std::endl;
  }

  VkSurfaceFormatKHR CFXSwapChain::chooseSwapSurfaceFormat(
      const std::vector<VkSurfaceFormatKHR> &availableFormats)
  {
    for (const auto &availableFormat : availableFormats)
    {
      if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM &&
          availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
      {
        return availableFormat;
      }
    }

    return availableFormats[0];
  }

  VkPresentModeKHR CFXSwapChain::chooseSwapPresentMode(
      const std::vector<VkPresentModeKHR> &availablePresentModes, int deviceIndex)
  {

    // for (const auto &availablePresentMode : availablePresentModes) {
    //   if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
    //     std::cout << "Present mode: immediate" << std::endl;
    //     return availablePresentMode;
    //   }
    // }
    
    return VK_PRESENT_MODE_IMMEDIATE_KHR;
  }

  VkExtent2D CFXSwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities)
  {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
      return capabilities.currentExtent;
    }
    else
    {
      VkExtent2D actualExtent = windowExtent;
      actualExtent.width = std::max(
          capabilities.minImageExtent.width,
          std::min(capabilities.maxImageExtent.width, actualExtent.width));
      actualExtent.height = std::max(
          capabilities.minImageExtent.height,
          std::min(capabilities.maxImageExtent.height, actualExtent.height));

      return actualExtent;
    }
  }

  VkFormat CFXSwapChain::findDepthFormat()
  {
    return device.findSupportedFormat(
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
  }
}