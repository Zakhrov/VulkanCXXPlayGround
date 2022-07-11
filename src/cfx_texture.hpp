#pragma once
#include "cfx_device.hpp"
#include <string>

namespace cfx{
    class Texture {
        public:
        Texture(int deviceIndex, CFXDevice &device, const std::string &filepath);
        ~Texture();
        VkSampler getSampler() {return sampler;}
        VkImageView getImageView() {return imageView;}
        VkImageLayout getImageLayout() {return imageLayout;}
        private:
        void transitionImageLayout(VkImageLayout oldImageLayout,VkImageLayout newImageLayout);
        void generateMipMaps();
        CFXDevice &cfxDevice;
        int width, height, mipLevels;
        VkImage image;
        VkImageView imageView;
        VkSampler sampler;
        VkDeviceMemory imageMemory;
        VkFormat imageFormat;
        VkImageLayout imageLayout;
        int deviceIndex;

    };
}