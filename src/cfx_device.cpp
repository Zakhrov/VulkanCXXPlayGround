#include "cfx_device.hpp"

// std headers
#include <cstring>
#include <iostream>
#include <set>
#include <unordered_set>

namespace cfx {

// local callback functions
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData) {
  std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

  return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugUtilsMessengerEXT *pDebugMessenger) {
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance,
      "vkCreateDebugUtilsMessengerEXT");
  if (func != nullptr) {
    return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
  } else {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

void DestroyDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks *pAllocator) {
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance,
      "vkDestroyDebugUtilsMessengerEXT");
  if (func != nullptr) {
    func(instance, debugMessenger, pAllocator);
  }
}

// class member functions
CFXDevice::CFXDevice(CFXWindow &window) : window{window} {
  createInstance();
  setupDebugMessenger();
  createDeviceGroups();
  createSurface();
  createLogicalDevice();
  
  createCommandPool();
  

}

CFXDevice::~CFXDevice() {
  for(VkCommandPool commandPool: commandPools){
    vkDestroyCommandPool(device_, commandPool, nullptr);
  }
  
  vkDestroyDevice(device_, nullptr);

  if (enableValidationLayers) {
    DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
  }

  for(VkSurfaceKHR surface_ : surfaces){
    vkDestroySurfaceKHR(instance, surface_, nullptr);
  }
  vkDestroyInstance(instance, nullptr);
}

void CFXDevice::createInstance() {
  if (enableValidationLayers && !checkValidationLayerSupport()) {
    throw std::runtime_error("validation layers requested, but not available!");
  }

  VkApplicationInfo appInfo = {};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = "VulkanProject App";
  appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.pEngineName = "No Engine";
  appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.apiVersion = VK_API_VERSION_1_2;

  VkInstanceCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pApplicationInfo = &appInfo;

  auto extensions = getRequiredExtensions();
  createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
  createInfo.ppEnabledExtensionNames = extensions.data();

  VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
  if (enableValidationLayers) {
    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();

    populateDebugMessengerCreateInfo(debugCreateInfo);
    createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
  } else {
    createInfo.enabledLayerCount = 0;
    createInfo.pNext = nullptr;
  }

  if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
    throw std::runtime_error("failed to create instance!");
  }

  hasGflwRequiredInstanceExtensions();
}

void CFXDevice::createDeviceGroups() {
  uint32_t deviceCount = 0;
  
  vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
  if (deviceCount == 0) {
    throw std::runtime_error("failed to find GPUs with Vulkan support!");
  }
  std::cout << "Device count: " << deviceCount << std::endl;
  physicalDevices.resize(deviceCount);
  deviceIds.resize(deviceCount);
  deviceNames.resize(deviceCount);
  deviceMasks.resize(deviceCount);
  deviceIndices.resize(deviceCount);
  
  vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data());



   if(vkEnumeratePhysicalDeviceGroups(instance,&deviceGroupCount,VK_NULL_HANDLE) != VK_SUCCESS){
      throw std::runtime_error("Failed to enumerate device group");


    }
    std::cout<< "DEVICE GROUPS " << deviceGroupCount << std::endl;
    physicalDeviceGroupProperties.resize(deviceGroupCount);
         for (int i = 0; i < deviceGroupCount; i++) {
      


      //  physicalDeviceGroupProperties[i] = pdProperties;
     
      physicalDeviceGroupProperties[i].sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES;
    std::cout << "DEV GROUP PROPS STYPE 2 "<< physicalDeviceGroupProperties[i].sType << std::endl;
    }
    
    if(vkEnumeratePhysicalDeviceGroups(instance,&deviceGroupCount,physicalDeviceGroupProperties.data()) != VK_SUCCESS){
      throw std::runtime_error("Failed to enumerate device group");

    }
    std::cout << "DEV GROUP PROPS SIZE "<< physicalDeviceGroupProperties.size() << std::endl;
    std::cout << "DEV GROUP PROPS  SUBSET ALLOCATION "<< physicalDeviceGroupProperties[0].subsetAllocation << std::endl;
    properties.resize(physicalDeviceGroupProperties[0].physicalDeviceCount);
    std::vector<VkPhysicalDeviceFeatures> physicalFeatures(physicalDeviceGroupProperties[0].physicalDeviceCount);
    std::vector<VkPhysicalDeviceFeatures2> physicalFeatures2(physicalDeviceGroupProperties[0].physicalDeviceCount);
    for(int i=0; i< physicalDeviceGroupProperties[0].physicalDeviceCount; i++){
      VkPhysicalDeviceSynchronization2FeaturesKHR syncFeature{};
      syncFeature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES_KHR;
      syncFeature.synchronization2 = true;
      physicalFeatures2[i].sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
      physicalFeatures2[i].pNext = &syncFeature;
      
      vkGetPhysicalDeviceProperties(physicalDeviceGroupProperties[0].physicalDevices[i], &properties[i]);
      vkGetPhysicalDeviceFeatures(physicalDeviceGroupProperties[0].physicalDevices[i],&physicalFeatures[i]);
      vkGetPhysicalDeviceFeatures2(physicalDeviceGroupProperties[0].physicalDevices[i],&physicalFeatures2[i]);
      deviceIds[i] = properties[i].deviceID;
      deviceMasks[i] = i+1;
      deviceIndices[i] = i;

      std::cout << properties[i].deviceName << std::endl;
      std::cout << properties[i].apiVersion << std::endl;
      deviceNames[i] = properties[i].deviceName;
      
      
      
      


    }

}

void CFXDevice::createLogicalDevice() {
  
  

   QueueFamilyIndices indices = findQueueFamilies(physicalDevices)[0];

  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
  std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily, indices.presentFamily};

  float queuePriority = 1.0f;
  for (uint32_t queueFamily : uniqueQueueFamilies) {
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = queueFamily;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;
    queueCreateInfos.push_back(queueCreateInfo);
  }

  VkPhysicalDeviceFeatures deviceFeatures = {};
  deviceFeatures.samplerAnisotropy = VK_TRUE;

  VkDeviceCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

  createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
  createInfo.pQueueCreateInfos = queueCreateInfos.data();

  createInfo.pEnabledFeatures = &deviceFeatures;
  createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
  createInfo.ppEnabledExtensionNames = deviceExtensions.data();

  // might not really be necessary anymore because device specific validation layers
  // have been deprecated
  if (enableValidationLayers) {
    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();
  } else {
    createInfo.enabledLayerCount = 0;
  }
     VkDeviceGroupDeviceCreateInfo deviceGroupInfo{};
    deviceGroupInfo.sType = VK_STRUCTURE_TYPE_DEVICE_GROUP_DEVICE_CREATE_INFO;
   for(int i=0;i<deviceGroupCount; i++){
      if (physicalDeviceGroupProperties[i].physicalDeviceCount > 1) {
        deviceGroupInfo.physicalDeviceCount = physicalDeviceGroupProperties[i].physicalDeviceCount;
        deviceGroupInfo.pPhysicalDevices = physicalDeviceGroupProperties[i].physicalDevices;
        VkDeviceMemoryOverallocationCreateInfoAMD memoryAllocationInfoAMD{};
        memoryAllocationInfoAMD.sType = VK_STRUCTURE_TYPE_DEVICE_MEMORY_OVERALLOCATION_CREATE_INFO_AMD;
        memoryAllocationInfoAMD.overallocationBehavior = VK_MEMORY_OVERALLOCATION_BEHAVIOR_ALLOWED_AMD;
        deviceGroupInfo.pNext = &memoryAllocationInfoAMD;
        // createInfo.pNext = &deviceGroupInfo;
        
        
        std::cout << "physical device group : " << deviceGroupInfo.physicalDeviceCount << std::endl;
    }
    else{
        std::cout<< "CANNOT MAKE CROSSFIRE :(" << std::endl;
    }
   }
   createInfo.pNext = &deviceGroupInfo;
   int discreteDeviceIndex = 0;

   



  if (vkCreateDevice(physicalDevices[0], &createInfo, nullptr, &device_) != VK_SUCCESS) {
    throw std::runtime_error("failed to create logical device!");
  }
  

  vkGetDeviceQueue(device_, indices.graphicsFamily, 0, &graphicsQueue);
  vkGetDeviceQueue(device_, indices.presentFamily, 0, &presentQueue);
  
  

  
}

void CFXDevice::createCommandPool() {
  
   QueueFamilyIndices queueFamilyIndices = findPhysicalQueueFamilies()[0];
  VkCommandPool commandPool;
  VkCommandPoolCreateInfo poolInfo = {};
  poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
  poolInfo.flags =
      VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

  if (vkCreateCommandPool(device_, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
    throw std::runtime_error("failed to create command pool!");
  }
  commandPools.push_back(commandPool);

  
}

void CFXDevice::createSurface() {
  std::cout << "Creating Surface" << std::endl;
  // for(VkPhysicalDevice device: physicalDevices){
    VkSurfaceKHR surface_;
  //   window.createWindowSurface(instance, &surface_);
  // surfaces.push_back(surface_);

  // }
   window.createWindowSurface(instance, &surface_); 
   surfaces.push_back(surface_);


   
 }


void CFXDevice::populateDebugMessengerCreateInfo(
    VkDebugUtilsMessengerCreateInfoEXT &createInfo) {
  createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  createInfo.pfnUserCallback = debugCallback;
  createInfo.pUserData = nullptr;  // Optional
}

void CFXDevice::setupDebugMessenger() {
  if (!enableValidationLayers) return;
  VkDebugUtilsMessengerCreateInfoEXT createInfo;
  populateDebugMessengerCreateInfo(createInfo);
  if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
    throw std::runtime_error("failed to set up debug messenger!");
  }
}

bool CFXDevice::checkValidationLayerSupport() {
  uint32_t layerCount;
  vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

  std::vector<VkLayerProperties> availableLayers(layerCount);
  vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

  for (const char *layerName : validationLayers) {
    bool layerFound = false;

    for (const auto &layerProperties : availableLayers) {
      if (strcmp(layerName, layerProperties.layerName) == 0) {
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

std::vector<const char *> CFXDevice::getRequiredExtensions() {
  uint32_t glfwExtensionCount = 0;
  const char **glfwExtensions;
  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

  if (enableValidationLayers) {
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }

  return extensions;
}

void CFXDevice::hasGflwRequiredInstanceExtensions() {
  uint32_t extensionCount = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
  std::vector<VkExtensionProperties> extensions(extensionCount);
  vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

  std::cout << "available extensions:" << std::endl;
  std::unordered_set<std::string> available;
  for (const auto &extension : extensions) {
    std::cout << "\t" << extension.extensionName << std::endl;
    available.insert(extension.extensionName);
  }

  std::cout << "required extensions:" << std::endl;
  auto requiredExtensions = getRequiredExtensions();
  for (const auto &required : requiredExtensions) {
    std::cout << "\t" << required << std::endl;
    if (available.find(required) == available.end()) {
      throw std::runtime_error("Missing required glfw extension");
    }
  }
}

bool CFXDevice::checkDeviceExtensionSupport(VkPhysicalDevice device) {
  uint32_t extensionCount;
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

  std::vector<VkExtensionProperties> availableExtensions(extensionCount);
  vkEnumerateDeviceExtensionProperties(
      device,
      nullptr,
      &extensionCount,
      availableExtensions.data());

  std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

  for (const auto &extension : availableExtensions) {
    requiredExtensions.erase(extension.extensionName);
  }

  return requiredExtensions.empty();
}

std::vector<QueueFamilyIndices> CFXDevice::findQueueFamilies(std::vector<VkPhysicalDevice> devices) {
  
  QueueFamilyIndices indices;

  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(devices[0], &queueFamilyCount, nullptr);

  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(devices[0], &queueFamilyCount, queueFamilies.data());

  int i = 0;
  for (const auto &queueFamily : queueFamilies) {
    if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      indices.graphicsFamily = i;
      indices.graphicsFamilyHasValue = true;
    }
    VkBool32 presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(devices[0], i, surfaces[0], &presentSupport);
    if (queueFamily.queueCount > 0 && presentSupport) {
      indices.presentFamily = i;
      indices.presentFamilyHasValue = true;
    }
    if (indices.isComplete()) {
      break;
    }

    i++;
  }

  return {indices};

}

std::vector<SwapChainSupportDetails> CFXDevice::querySwapChainSupport(std::vector<VkPhysicalDevice> devices) {
  
    SwapChainSupportDetails details;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(devices[0], surfaces[0], &details.capabilities);

  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(devices[0], surfaces[0], &formatCount, nullptr);

  if (formatCount != 0) {
    details.formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(devices[0], surfaces[0], &formatCount, details.formats.data());
  }

  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(devices[0], surfaces[0], &presentModeCount, nullptr);

  if (presentModeCount != 0) {
    details.presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        devices[0],
        surfaces[0],
        &presentModeCount,
        details.presentModes.data());
  }
  return {details};

}

VkFormat CFXDevice::findSupportedFormat(
    const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
  for (VkFormat format : candidates) {
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(physicalDevices[0], format, &props);

    if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
      return format;
    } else if (
        tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
      return format;
    }
  }
  throw std::runtime_error("failed to find supported format!");
}

uint32_t CFXDevice::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties,int deviceIndex) {
  VkPhysicalDeviceMemoryProperties memProperties;
  vkGetPhysicalDeviceMemoryProperties(physicalDevices[deviceIndex], &memProperties);
  
  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
    if ((typeFilter & (1 << i)) &&
        (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
          VkPeerMemoryFeatureFlags memoryFlags{};
          vkGetDeviceGroupPeerMemoryFeatures(device_,i, deviceIndex !=0 ? deviceIndices[deviceIndex -1]:deviceIndices[deviceIndex],deviceIndex ==0 ? deviceIndices[deviceIndex+1] : deviceIndices[deviceIndex],&memoryFlags);
          memProperties.memoryHeaps[i].flags = memoryFlags;
          
      return i;
    }
  }

  throw std::runtime_error("failed to find suitable memory type!");
}

void CFXDevice::createBuffer(
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkBuffer &buffer,
    VkDeviceMemory &bufferMemory) {

      
  int deviceIndex = 0;
  int bindInfoCount = getDevicesinDeviceGroup();
  std::vector<VkBindBufferMemoryInfo> bufferMemoryInfos;
  bufferMemoryInfos.resize(bindInfoCount);
  for(int i =0; i < getDevicesinDeviceGroup();i++){

    VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = size;
  bufferInfo.usage = usage;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateBuffer(device_, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
    throw std::runtime_error("failed to create vertex buffer!");
  }

  VkMemoryRequirements memRequirements{};
  vkGetBufferMemoryRequirements(device_, buffer, &memRequirements);
  
  
  
  VkMemoryAllocateFlagsInfo allocFlagsInfo{};
  allocFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
  allocFlagsInfo.flags =  VK_MEMORY_ALLOCATE_DEVICE_MASK_BIT;
  allocFlagsInfo.deviceMask = deviceMasks[i];

    VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties,i);
  if(getDevicesinDeviceGroup() > 1){
  allocInfo.pNext = &allocFlagsInfo;
  }
  else{
    allocInfo.pNext = nullptr;
  }
  

  if (vkAllocateMemory(device_, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate vertex buffer memory!");
  }

  // vkBindBufferMemory(device_, buffer, bufferMemory, 0);
  VkBindBufferMemoryDeviceGroupInfo bindBufferMemoryDeviceGroupInfo{};
  bindBufferMemoryDeviceGroupInfo.sType = VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_DEVICE_GROUP_INFO;
  bindBufferMemoryDeviceGroupInfo.deviceIndexCount = deviceIndices.size();
  bindBufferMemoryDeviceGroupInfo.pDeviceIndices = deviceIndices.data();
  VkBindBufferMemoryInfo memoryInfo{};
  memoryInfo.sType = VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO;
  memoryInfo.buffer = buffer;
  memoryInfo.memory = bufferMemory;
  memoryInfo.memoryOffset = 0;
  if(getDevicesinDeviceGroup() > 1){
    memoryInfo.pNext = &bindBufferMemoryDeviceGroupInfo;
  }
  else{
    memoryInfo.pNext = nullptr;
  }
  
  bufferMemoryInfos[i] = memoryInfo;    
  }
  if(vkBindBufferMemory2(device_,bindInfoCount,bufferMemoryInfos.data())!=VK_SUCCESS){
    throw std::runtime_error("failed to bind Buffer memory 2 !");
  }else{
    std::cout << "Bind Buffer MEMORY 2 succeeded" <<std::endl;
  }
  
      
  
}

std::vector<VkCommandBuffer> CFXDevice::beginSingleTimeCommands() {
 

  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = commandPools[0];
  allocInfo.commandBufferCount = 1;

  VkCommandBuffer commandBuffer;
  vkAllocateCommandBuffers(device_, &allocInfo, &commandBuffer);

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(commandBuffer, &beginInfo);
  return {commandBuffer};
  
}

void CFXDevice::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
 
  vkEndCommandBuffer(commandBuffer);

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;

  vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(graphicsQueue);

  vkFreeCommandBuffers(device_, commandPools[0], 1, &commandBuffer);


  
}

void CFXDevice::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
  VkCommandBuffer commandBuffer = beginSingleTimeCommands()[0];
  //  for(VkCommandBuffer commandBuffer: commandBuffers){
     
  //  }
   VkBufferCopy copyRegion{};
  copyRegion.srcOffset = 0;  // Optional
  copyRegion.dstOffset = 0;  // Optional
  copyRegion.size = size;
  vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

  

  endSingleTimeCommands(commandBuffer);
}

void CFXDevice::copyBufferToImage(
     VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount) {
  VkCommandBuffer commandBuffer = beginSingleTimeCommands()[0];

  VkBufferImageCopy region{};
  region.bufferOffset = 0;
  region.bufferRowLength = 0;
  region.bufferImageHeight = 0;

  region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.mipLevel = 0;
  region.imageSubresource.baseArrayLayer = 0;
  region.imageSubresource.layerCount = layerCount;

  region.imageOffset = {0, 0, 0};
  region.imageExtent = {width, height, 1};

  vkCmdCopyBufferToImage(
      commandBuffer,
      buffer,
      image,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      1,
      &region);
  endSingleTimeCommands(commandBuffer);
}

void CFXDevice::createImageWithInfo(
    const VkImageCreateInfo &imageInfo,
    VkMemoryPropertyFlags properties,
    VkImage &image,
    VkDeviceMemory &imageMemory) {

      int bindImageMemoryCount = deviceIndices.size();
      std::vector<VkBindImageMemoryInfo> imageMemoryInfos;
      imageMemoryInfos.resize(bindImageMemoryCount);


      for(int i=0; i< deviceMasks.size();i++){
        if (vkCreateImage(device_, &imageInfo, nullptr, &image) != VK_SUCCESS) {
    throw std::runtime_error("failed to create image!");
  }

  VkMemoryRequirements memRequirements;
  vkGetImageMemoryRequirements(device_, image, &memRequirements);
  VkMemoryAllocateFlagsInfo allocFlagsInfo{};
  allocFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
  allocFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_MASK_BIT;
  allocFlagsInfo.deviceMask = deviceMasks[i];

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties,i);
  if(getDevicesinDeviceGroup() > 1){
    allocInfo.pNext = &allocFlagsInfo;
  }
  else{
    allocInfo.pNext = nullptr;
  }
  

  if (vkAllocateMemory(device_, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate image memory!");
  }

  // if (vkBindImageMemory(device_, image, imageMemory, 0) != VK_SUCCESS) {
  //   throw std::runtime_error("failed to bind image memory!");
  // }
  VkBindImageMemoryDeviceGroupInfo bindImageMemoryDeviceGroupInfo{};
  bindImageMemoryDeviceGroupInfo.sType = VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_DEVICE_GROUP_INFO;
  bindImageMemoryDeviceGroupInfo.deviceIndexCount = deviceIndices.size();
  bindImageMemoryDeviceGroupInfo.pDeviceIndices = deviceIndices.data();
  VkBindImageMemoryInfo memoryInfo{};
  memoryInfo.sType = VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO;
  memoryInfo.image = image;
  memoryInfo.memory = imageMemory;
  memoryInfo.memoryOffset = 0;
  memoryInfo.pNext = &bindImageMemoryDeviceGroupInfo;
  
  imageMemoryInfos[i] = memoryInfo;    

      
  }
  if(vkBindImageMemory2(device_,bindImageMemoryCount,imageMemoryInfos.data())!=VK_SUCCESS){
    throw std::runtime_error("failed to bind image memory 2!");
  }
      
  
  
}

} 