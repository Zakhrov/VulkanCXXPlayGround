#include "cfx_descriptors.hpp"
 
// std
#include <cassert>
#include <stdexcept>
#include <iostream>
 
namespace cfx {
 
// *************** Descriptor Set Layout Builder *********************
 
CFXDescriptorSetLayout::Builder &CFXDescriptorSetLayout::Builder::addBinding(
    uint32_t binding,
    VkDescriptorType descriptorType,
    VkShaderStageFlags stageFlags,
    uint32_t count) {
  assert(bindings.count(binding) == 0 && "Binding already in use");
  VkDescriptorSetLayoutBinding layoutBinding{};
  layoutBinding.binding = binding;
  layoutBinding.descriptorType = descriptorType;
  layoutBinding.descriptorCount = count;
  layoutBinding.stageFlags = stageFlags;
  bindings[binding] = layoutBinding;
  return *this;
}
 
std::unique_ptr<CFXDescriptorSetLayout> CFXDescriptorSetLayout::Builder::build(int deviceIndex) const {
  
  return std::make_unique<CFXDescriptorSetLayout>(cfxDevice, bindings,deviceIndex);
}

// *************** Descriptor Set Layout *********************
 
CFXDescriptorSetLayout::CFXDescriptorSetLayout(
    CFXDevice &cfxDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings,int deviceIndex)
    : cfxDevice{cfxDevice}, bindings{bindings}, layoutDeviceIndex{deviceIndex} {
      
  std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
  for (auto kv : bindings) {
    setLayoutBindings.push_back(kv.second);
  }
 
  VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
  descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
  descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();
  
 
  if (vkCreateDescriptorSetLayout(
          cfxDevice.device(layoutDeviceIndex),
          &descriptorSetLayoutInfo,
          nullptr,
          &descriptorSetLayout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create descriptor set layout!");
  }
  else{
    
  }
}
 
CFXDescriptorSetLayout::~CFXDescriptorSetLayout() {
  vkDestroyDescriptorSetLayout(cfxDevice.device(layoutDeviceIndex), descriptorSetLayout, nullptr);
}

// *************** Descriptor Pool Builder *********************
 
CFXDescriptorPool::Builder &CFXDescriptorPool::Builder::addPoolSize(
    VkDescriptorType descriptorType, uint32_t count) {
  poolSizes.push_back({descriptorType, count});
  return *this;
}
 
CFXDescriptorPool::Builder &CFXDescriptorPool::Builder::setPoolFlags(
    VkDescriptorPoolCreateFlags flags) {
  poolFlags = flags;
  return *this;
}
CFXDescriptorPool::Builder &CFXDescriptorPool::Builder::setMaxSets(uint32_t count) {
  maxSets = count;
  return *this;
}
 
std::unique_ptr<CFXDescriptorPool> CFXDescriptorPool::Builder::build(int deviceIndex) const {
  return std::make_unique<CFXDescriptorPool>(cfxDevice, maxSets, poolFlags, poolSizes,deviceIndex);
}
 

 // *************** Descriptor Pool *********************
 
CFXDescriptorPool::CFXDescriptorPool(
    CFXDevice &cfxDevice,
    uint32_t maxSets,
    VkDescriptorPoolCreateFlags poolFlags,
    const std::vector<VkDescriptorPoolSize> &poolSizes,int deviceIndex)
    : cfxDevice{cfxDevice}, poolDeviceIndex{deviceIndex} {
  VkDescriptorPoolCreateInfo descriptorPoolInfo{};
  descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
  descriptorPoolInfo.pPoolSizes = poolSizes.data();
  descriptorPoolInfo.maxSets = maxSets;
  descriptorPoolInfo.flags = poolFlags;
  
   if (vkCreateDescriptorPool(cfxDevice.device(poolDeviceIndex), &descriptorPoolInfo, nullptr, &descriptorPool) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create descriptor pool!");
  }

 
  
}
 
CFXDescriptorPool::~CFXDescriptorPool() {
  vkDestroyDescriptorPool(cfxDevice.device(poolDeviceIndex), descriptorPool, nullptr);
  
}
 
bool CFXDescriptorPool::allocateDescriptorSet(
    const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptorSet, int deviceIndex) const {
  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = descriptorPool;
  allocInfo.pSetLayouts = &descriptorSetLayout;
  allocInfo.descriptorSetCount = 1;
  
 
  // Might want to create a "DescriptorPoolManager" class that handles this case, and builds
  // a new pool whenever an old pool fills up. But this is beyond our current scope
  if (vkAllocateDescriptorSets(cfxDevice.device(deviceIndex), &allocInfo, &descriptorSet) != VK_SUCCESS) {
    std::cout << "CANNOT ALLOCATE DESCRIPTOR SET " << std::endl;
    return false;
  }
  return true;
}
 
void CFXDescriptorPool::freeDescriptors(std::vector<VkDescriptorSet> descriptorSets,int deviceIndex) const {
  vkFreeDescriptorSets(
      cfxDevice.device(deviceIndex),
      descriptorPool,
      descriptorSets.size(),
      descriptorSets.data());
}
 
void CFXDescriptorPool::resetPool() {
  vkResetDescriptorPool(cfxDevice.device(poolDeviceIndex), descriptorPool, 0);
}


// *************** Descriptor Writer *********************
 
CFXDescriptorWriter::CFXDescriptorWriter(CFXDescriptorSetLayout &setLayout, CFXDescriptorPool &pool)
    : setLayout{setLayout}, pool{pool} {}
 
CFXDescriptorWriter &CFXDescriptorWriter::writeBuffer(
    uint32_t binding, VkDescriptorBufferInfo *bufferInfo) {
  assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");
 
  auto &bindingDescription = setLayout.bindings[binding];
 
  assert(
      bindingDescription.descriptorCount == 1 &&
      "Binding single descriptor info, but binding expects multiple");
 
  VkWriteDescriptorSet write{};
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.descriptorType = bindingDescription.descriptorType;
  write.dstBinding = binding;
  write.pBufferInfo = bufferInfo;
  write.descriptorCount = 1;
 
  writes.push_back(write);
  return *this;
}
 
CFXDescriptorWriter &CFXDescriptorWriter::writeImage(
    uint32_t binding, VkDescriptorImageInfo *imageInfo) {
  assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");
 
  auto &bindingDescription = setLayout.bindings[binding];
 
  assert(
      bindingDescription.descriptorCount == 1 &&
      "Binding single descriptor info, but binding expects multiple");
 
  VkWriteDescriptorSet write{};
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.descriptorType = bindingDescription.descriptorType;
  write.dstBinding = binding;
  write.pImageInfo = imageInfo;
  write.descriptorCount = 1;
 
  writes.push_back(write);
  return *this;
}
 
bool CFXDescriptorWriter::build(VkDescriptorSet &descriptorSet, int deviceIndex ) {
  bool success = pool.allocateDescriptorSet(setLayout.getDescriptorSetLayout(),descriptorSet,deviceIndex);
  if (!success) {
    return false;
  }
  overwrite(descriptorSet,deviceIndex );
  return true;
}
 
void CFXDescriptorWriter::overwrite(VkDescriptorSet &descriptorSet,int deviceIndex ) {
  for (auto &write : writes) {
   write.dstSet = descriptorSet;
    
  }
  vkUpdateDescriptorSets(pool.cfxDevice.device(deviceIndex), writes.size(), writes.data(), 0, nullptr);
}

}