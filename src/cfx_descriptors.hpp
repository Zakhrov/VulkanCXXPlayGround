#pragma once
 
#include "cfx_device.hpp"
 
// std
#include <memory>
#include <unordered_map>
#include <vector>
 
namespace cfx {
 
class CFXDescriptorSetLayout {
 public:
  class Builder {
   public:
    Builder(CFXDevice &cfxDevice) : cfxDevice{cfxDevice} {}
 
    Builder &addBinding(
        uint32_t binding,
        VkDescriptorType descriptorType,
        VkShaderStageFlags stageFlags,
        uint32_t count = 1);
    std::unique_ptr<CFXDescriptorSetLayout> build(int deviceIndex) const;
 
   private:
    CFXDevice &cfxDevice;
    std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
  };
 
  CFXDescriptorSetLayout(
      CFXDevice &cfxDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings,int deviceIndex);
  ~CFXDescriptorSetLayout();
  CFXDescriptorSetLayout(const CFXDescriptorSetLayout &) = delete;
  CFXDescriptorSetLayout &operator=(const CFXDescriptorSetLayout &) = delete;
 
  VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }
 
 private:
  CFXDevice &cfxDevice;
  VkDescriptorSetLayout descriptorSetLayout;
  std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;
  int layoutDeviceIndex;
 
  friend class CFXDescriptorWriter;
};
 
class CFXDescriptorPool {
 public:
  class Builder {
   public:
    Builder(CFXDevice &cfxDevice) : cfxDevice{cfxDevice} {}
 
    Builder &addPoolSize(VkDescriptorType descriptorType, uint32_t count);
    Builder &setPoolFlags(VkDescriptorPoolCreateFlags flags);
    Builder &setMaxSets(uint32_t count);
    std::unique_ptr<CFXDescriptorPool> build(int deviceIndex) const;
 
   private:
    CFXDevice &cfxDevice;
    std::vector<VkDescriptorPoolSize> poolSizes{};
    uint32_t maxSets = 1000;
    VkDescriptorPoolCreateFlags poolFlags = 0;
  };
 
  CFXDescriptorPool(
      CFXDevice &cfxDevice,
      uint32_t maxSets,
      VkDescriptorPoolCreateFlags poolFlags,
      const std::vector<VkDescriptorPoolSize> &poolSizes,int deviceIndex);
  ~CFXDescriptorPool();
  CFXDescriptorPool(const CFXDescriptorPool &) = delete;
  CFXDescriptorPool &operator=(const CFXDescriptorPool &) = delete;
 
  bool allocateDescriptorSet(
      const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptorSet, int deviceIndex) const;
 
  void freeDescriptors(std::vector<VkDescriptorSet> descriptorSets, int deviceIndex) const;
 
  void resetPool();
 
 private:
  CFXDevice &cfxDevice;
  VkDescriptorPool descriptorPool;
  int poolDeviceIndex;
 
  friend class CFXDescriptorWriter;
};
 
class CFXDescriptorWriter {
 public:
  CFXDescriptorWriter(CFXDescriptorSetLayout &setLayout, CFXDescriptorPool &pool);
 
  CFXDescriptorWriter &writeBuffer(uint32_t binding, VkDescriptorBufferInfo *bufferInfo);
  CFXDescriptorWriter &writeImage(uint32_t binding, VkDescriptorImageInfo *imageInfo);
 
  bool build(VkDescriptorSet &set,int deviceIndex);
  void overwrite(VkDescriptorSet &set, int deviceIndex);
 
 private:
  CFXDescriptorSetLayout &setLayout;
  CFXDescriptorPool &pool;
  std::vector<VkWriteDescriptorSet> writes;
};
 
}  // namespace cfx