#include "cfx_renderer.hpp"
#include<stdexcept>
#include<array>
#include<iostream>
#include<memory>


namespace cfx{
  
    Renderer::Renderer(CFXWindow &window, CFXDevice &device) : cfxWindow{window}, cfxDevice{device} {
        
        recreateSwapChain();
        createCommandBuffers();
    }
    Renderer::~Renderer(){
        freeCommandBuffers();
    }
    
    void Renderer::recreateSwapChain(){
        std::cout << "CREATE SWAPCHAIN"<< std::endl;
        auto extent = cfxWindow.getExtent();
        while(extent.width == 0 || extent.height == 0){
            extent = cfxWindow.getExtent();
            glfwWaitEvents();
        }
        vkDeviceWaitIdle(cfxDevice.device());
        if(cfxSwapChain == nullptr){
            cfxSwapChain = std::make_unique<CFXSwapChain>(cfxDevice,extent);
        }
        else{
            std::shared_ptr<CFXSwapChain> oldSwapchain = std::move(cfxSwapChain); 
            cfxSwapChain = std::make_unique<CFXSwapChain>(cfxDevice,extent,oldSwapchain);
            if(!oldSwapchain->compareSwapFormats(*cfxSwapChain.get())){
                throw std::runtime_error("Swapchain Image or Depth format changed");
            }
            // if(cfxSwapChain->imageCount() != commandBuffers.size()){
            //     freeCommandBuffers();
            //     createCommandBuffers();
            // }
        
        }
    }

     void Renderer::createCommandBuffers(){
        //  commandBuffers.resize(cfxSwapChain->imageCount());
        //  std::cout << "COMMAND BUFFER SIZE  " << commandBuffers.size() << std::endl;
        //  VkCommandBufferAllocateInfo allocInfo{};
        //  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        //  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        //  allocInfo.commandPool = cfxDevice.getCommandPool().front();
        //  allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

        //  if(vkAllocateCommandBuffers(cfxDevice.device(),&allocInfo,commandBuffers.data()) != VK_SUCCESS){
        //      throw std::runtime_error("failed to allocate command buffers");
        //  }

      commandBuffers.resize(CFXSwapChain::MAX_FRAMES_IN_FLIGHT);

  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = cfxDevice.getCommandPool()[0];
  allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

  if (vkAllocateCommandBuffers(cfxDevice.device(), &allocInfo, commandBuffers.data()) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to allocate command buffers!");
  }
  
  
        

   }
   void Renderer::freeCommandBuffers(){
       vkFreeCommandBuffers(cfxDevice.device(), cfxDevice.getCommandPool()[0], static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
   }


  
   
    VkCommandBuffer Renderer::beginFrame(){
        assert(!isFrameStarted && "Cant call beginFrame while frame is in progress");
            auto result = cfxSwapChain->acquireNextImage(&currentImageIndex);
            if(result == VK_ERROR_OUT_OF_DATE_KHR){
                recreateSwapChain();
                return nullptr;
            }

            if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR){
                throw std::runtime_error("failed to aquire swap chain image");
            }

            isFrameStarted = true;
            auto commandBuffer = getCurrentCommandBuffer();
            VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
       

        if(cfxDevice.getDevicesinDeviceGroup() > 1 ){
            if(currentImageIndex % 2 == 0){
                deviceIndex = 1;
            }
            else{
                deviceIndex = 2;
            }
            // std::cout << "PROCESSING FRAME " << currentFrameIndex << " PROCESSING IMAGE " << currentImageIndex << std::endl;
            // std::cout << "VULKAN DEVICE INDEX BEGIN " << cfxDevice.getDeviceName(deviceIndex -1) << std::endl;
          VkDeviceGroupCommandBufferBeginInfo deviceGroupCommandBufferInfo{};
        deviceGroupCommandBufferInfo.sType = VK_STRUCTURE_TYPE_DEVICE_GROUP_COMMAND_BUFFER_BEGIN_INFO;
        deviceGroupCommandBufferInfo.deviceMask =  deviceIndex ;
        // VkDeviceGroupRenderPassBeginInfo deviceGroupRenderPassInfo{};
        //      deviceGroupRenderPassInfo.sType = VK_STRUCTURE_TYPE_DEVICE_GROUP_RENDER_PASS_BEGIN_INFO;
        //      deviceGroupRenderPassInfo.deviceMask = deviceIndex;
        //      deviceGroupRenderPassInfo.deviceRenderAreaCount = cfxDevice.getDeviceRects().size();
        //      deviceGroupRenderPassInfo.pDeviceRenderAreas = cfxDevice.getDeviceRects().data();
            
            
        beginInfo.pNext = &deviceGroupCommandBufferInfo;

        }
        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
                    throw std::runtime_error("failed to begin recording command buffer!");
          }
        return commandBuffer;

    }
    void Renderer::endFrame(){
        assert(isFrameStarted && "cant call endFrame while frame is not in progress");
        auto commandBuffer = getCurrentCommandBuffer();
         
          if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
               throw std::runtime_error("failed to record command buffer!");
          }
        //   std::cout << "VULKAN DEVICE INDEX END " << cfxDevice.getDeviceName(deviceIndex -1) << std::endl;

          VkResult result = cfxSwapChain->submitCommandBuffers(&commandBuffer,&currentImageIndex,deviceIndex);
            if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || cfxWindow.wasWindowResized()){
                cfxWindow.restWindowResizedFlag();
                recreateSwapChain();

            }
            else if(result != VK_SUCCESS){
                throw std::runtime_error("failed to present swap chain image");
            }

            isFrameStarted = false;
            currentFrameIndex = (currentFrameIndex + 1) % CFXSwapChain::MAX_FRAMES_IN_FLIGHT;



    }
    void Renderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer){
        assert(isFrameStarted && "Cant call beginSwapChainRenderPass if frame is not in progress");
        assert(commandBuffer == getCurrentCommandBuffer() && "cant begin renderpass on a command buffer from a different frame");


        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = cfxSwapChain->getRenderPass();
        renderPassInfo.framebuffer = cfxSwapChain->getFrameBuffer(currentImageIndex);
          // renderPassInfo.pNext = &deviceGroupRenderPassInfo;

        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = cfxSwapChain->getSwapChainExtent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

      vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    //   std::cout << "BEGIN RENDER PASS" << std::endl;

      VkViewport viewport{};
      viewport.x = 0.0f;
      viewport.y = 0.0f;
      viewport.width = static_cast<float>(cfxSwapChain->getSwapChainExtent().width);
      viewport.height = static_cast<float>(cfxSwapChain->getSwapChainExtent().height);
      viewport.minDepth = 0.0f;
      viewport.maxDepth = 1.0f;
      VkRect2D scissor{{0, 0}, cfxSwapChain->getSwapChainExtent()};
      // std::cout << "SET VIEWPORT" << std::endl;
      vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
      // std::cout << "VIEWPORT SET" << std::endl;
      // std::cout << "SET SCISSOR" << std::endl;
      vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
      // std::cout << "SCISSOR SET" << std::endl;
      // std::cout << "RENDER GAME OBJECT" << std::endl;


    }
    void Renderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer){
        assert(isFrameStarted && "Cant call endSwapChainRenderPass if frame is not in progress");
        assert(commandBuffer == getCurrentCommandBuffer() && "cant end renderpass on a command buffer from a different frame");
        vkCmdEndRenderPass(commandBuffer);
        //   std::cout << "END RENDER PASS" << std::endl;

    }
    
  
  
}