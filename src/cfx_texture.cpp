#include "cfx_texture.hpp"
#include "cfx_buffer.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "external/stb/stb_image.h"
#include <stdexcept>
#include <iostream>

namespace cfx{
    Texture::Texture(int deviceIndex, CFXDevice &device, const std::string &filepath) : cfxDevice(device), deviceIndex(deviceIndex){
        
        int channels;
        int bytesPerPixel;
        

        stbi_uc *data = stbi_load(filepath.c_str(),&width,&height,&bytesPerPixel,4);
        mipLevels = std::floor(std::log2(std::max(width,height))) + 1;
        CFXBuffer stagingBuffer{cfxDevice,4,static_cast<uint32_t>(width * height),
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        deviceIndex};
        stagingBuffer.map();
        stagingBuffer.writeToBuffer(data);
        
        imageFormat = VK_FORMAT_R8G8B8A8_SRGB;
        

        VkImageCreateInfo imageInfo = {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.format = imageFormat;
        imageInfo.mipLevels = mipLevels;
        imageInfo.arrayLayers = 1;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.extent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1};
        imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;


        std::cout << "CREATING TEXTURE IMAGE INFO ON DEVICE " << cfxDevice.getDeviceName(deviceIndex) <<  std::endl;
        cfxDevice.createImageWithInfo(imageInfo,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,image,imageMemory,deviceIndex);

        std::cout << "START TRANSITION FROM  VK_IMAGE_LAYOUT_UNDEFINED to VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL " << std::endl;
        transitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        std::cout << "END TRANSITION FROM  VK_IMAGE_LAYOUT_UNDEFINED to VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL " << std::endl;


        std::cout << "START COPY BUFFER TO IMAGE DEVICE INDEX " << cfxDevice.getDeviceName(deviceIndex) << std::endl;
        cfxDevice.copyBufferToImage(stagingBuffer.getBuffer(),image,static_cast<uint32_t>(width),static_cast<uint32_t>(height),1,deviceIndex);
        std::cout << "END COPY BUFFER TO IMAGE DEVICE INDEX " << cfxDevice.getDeviceName(deviceIndex) << std::endl;

        
        
        generateMipMaps();
        std::cout << "END GENERATE MIPMAPS "<< std::endl;
        // transitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        
        imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = static_cast<float>(mipLevels);
        samplerInfo.maxAnisotropy = 4.0f;
        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

        std::cout << "CREATING SAMPLER ON DEVICE " << cfxDevice.getDeviceName(deviceIndex) << std::endl;

        vkCreateSampler(cfxDevice.device(deviceIndex),&samplerInfo,nullptr,&sampler);
        std::cout << "CREATED SAMPLER ON DEVICE " << cfxDevice.getDeviceName(deviceIndex) << std::endl;

        VkImageViewCreateInfo imageViewInfo{};
        imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewInfo.format = imageFormat;
        imageViewInfo.components = {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B,VK_COMPONENT_SWIZZLE_A};
        imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewInfo.subresourceRange.baseArrayLayer = 0;
        imageViewInfo.subresourceRange.baseMipLevel = 0;
        imageViewInfo.subresourceRange.layerCount = 1;
        imageViewInfo.subresourceRange.levelCount = mipLevels;
        imageViewInfo.image = image;

        std::cout << "CREATING TEXTURE IMAGE VIEW" << std::endl;

        vkCreateImageView(cfxDevice.device(deviceIndex),&imageViewInfo,nullptr,&imageView);
        std::cout << "CREATED TEXTURE IMAGE VIEW" << std::endl;
        stbi_image_free(data);

    }
    void Texture::generateMipMaps(){
        std::cout << "GENERATING MIPMAPS " << std::endl;
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(cfxDevice.getPhysicalDevices()[deviceIndex],imageFormat,&formatProperties);
        if(!formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT){
            throw std::runtime_error("Texture Image format does not support linear blitting");
        }
        VkCommandBuffer commandBuffer = cfxDevice.beginSingleTimeCommands(deviceIndex);


        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.subresourceRange.levelCount = 1;
        int32_t mipWidth = width;
        int32_t mipHeight = height;

        for(uint32_t i = 1; i < mipLevels; i++){
            std::cout << std::endl;
            barrier.subresourceRange.baseMipLevel = i - 1;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            std::cout << "BEGIN vkCmdPipelineBarrier VK_PIPELINE_STAGE_TRANSFER_BIT for level " << i << std::endl << std::endl;
            vkCmdPipelineBarrier(commandBuffer,VK_PIPELINE_STAGE_TRANSFER_BIT,VK_PIPELINE_STAGE_TRANSFER_BIT,0,0,nullptr,0,nullptr,1,&barrier);
            std::cout << "END vkCmdPipelineBarrier VK_PIPELINE_STAGE_TRANSFER_BIT for level " << i << std::endl << std::endl;
            VkImageBlit blit{};
            blit.srcOffsets[0] = {0,0,0};
            blit.srcOffsets[1] = {mipWidth, mipHeight, 1};
            blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.srcSubresource.mipLevel = i - 1;
            blit.srcSubresource.baseArrayLayer = 0;
            blit.srcSubresource.layerCount = 1;
            blit.dstOffsets[0] = {0,0,0};
            blit.dstOffsets[1] = {mipWidth > 1 ? mipWidth / 2 : 1,mipHeight > 1 ? mipHeight / 2 : 1,1};
            blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.dstSubresource.mipLevel = i;
            blit.dstSubresource.baseArrayLayer = 0;
            blit.dstSubresource.layerCount = 1;

            std::cout << "BEGIN vkCmdBlitImage VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL to  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL for level " << i << std::endl << std::endl;
            vkCmdBlitImage(commandBuffer,image,VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,image,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,1,&blit,VK_FILTER_LINEAR);
            std::cout << "END vkCmdBlitImage VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL to  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL for level " << i << std::endl << std::endl;

            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            std::cout << "BEGIN vkCmdPipelineBarrier VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT for level " << i << std::endl << std::endl;
            vkCmdPipelineBarrier(commandBuffer,VK_PIPELINE_STAGE_TRANSFER_BIT,VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,0,0,nullptr,0,nullptr,1,&barrier);
            std::cout << "END vkCmdPipelineBarrier VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT for level " << i << std::endl << std::endl;
            if(mipWidth > 1) mipWidth = mipWidth / 2;
            if(mipHeight > 1) mipHeight = mipHeight / 2;


        }
        barrier.subresourceRange.baseMipLevel = mipLevels - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        std::cout << "BEGIN vkCmdPipelineBarrier VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT for level " << "OUTSIDE MIPLEVEL LOOP" << std::endl << std::endl;
        vkCmdPipelineBarrier(commandBuffer,VK_PIPELINE_STAGE_TRANSFER_BIT,VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,0,0,nullptr,0,nullptr,1,&barrier);
        std::cout << "END vkCmdPipelineBarrier VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT for level " << "OUTSIDE MIPLEVEL LOOP" << std::endl << std::endl;
        std::cout << "BEGIN END SINGLETIME COMMANDS FOR " << cfxDevice.getDeviceName(deviceIndex) << std::endl << std::endl;
        cfxDevice.endSingleTimeCommands(commandBuffer,deviceIndex);
        std::cout << "END END SINGLETIME COMMANDS FOR " << cfxDevice.getDeviceName(deviceIndex) << std::endl << std::endl;

        
    }
    void Texture::transitionImageLayout(VkImageLayout oldLayout,VkImageLayout newLayout){
        VkCommandBuffer commandBuffer = cfxDevice.beginSingleTimeCommands(deviceIndex);
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.subresourceRange.levelCount = mipLevels;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if(oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL){
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;


        }
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL){

            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

        }
        else{
            throw std::runtime_error("unsupported layout transition");
        }
        vkCmdPipelineBarrier(commandBuffer,sourceStage,destinationStage,0,0,nullptr,0,nullptr,1,&barrier);
        cfxDevice.endSingleTimeCommands(commandBuffer,deviceIndex);


    }
    Texture::~Texture(){
        vkDestroyImage(cfxDevice.device(deviceIndex),image,nullptr);
        vkFreeMemory(cfxDevice.device(deviceIndex),imageMemory,nullptr);
        vkDestroyImageView(cfxDevice.device(deviceIndex),imageView,nullptr);
        vkDestroySampler(cfxDevice.device(deviceIndex),sampler,nullptr);

    }
}