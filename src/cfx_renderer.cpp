#include "cfx_renderer.hpp"
#include<stdexcept>
#include<array>
#include<iostream>
#include<memory>


namespace cfx{
  
    Renderer::Renderer(CFXWindow &window, CFXDevice &device) : cfxWindow{window}, cfxDevice{device} {
        deviceCount = cfxDevice.getDevicesinDeviceGroup();
        commandBuffers.resize(deviceCount);
        recreateSwapChain();

        for(int i = 0; i < deviceCount; i++){
            createCommandBuffers(i);
        }
        
    }
    Renderer::~Renderer(){
        for(int i = 0; i < deviceCount; i++){
            freeCommandBuffers(i);
        }
    }
    
    void Renderer::recreateSwapChain(){
        std::cout << "CREATE SWAPCHAIN"<< std::endl;
        auto extent = cfxWindow.getExtent();
        while(extent.width == 0 || extent.height == 0){
            extent = cfxWindow.getExtent();
            glfwWaitEvents();
        }
        for(int i = 0; i < deviceCount; i++){
            vkDeviceWaitIdle(cfxDevice.device(i));
        }
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

     void Renderer::createCommandBuffers(int deviceIndex){
       

      commandBuffers[deviceIndex].resize(CFXSwapChain::MAX_FRAMES_IN_FLIGHT);

  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = cfxDevice.getCommandPool(deviceIndex);
  allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());
  if (vkAllocateCommandBuffers(cfxDevice.device(deviceIndex), &allocInfo, commandBuffers[deviceIndex].data()) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to allocate command buffers!");
  }
  
        

   }
   void Renderer::freeCommandBuffers(int deviceIndex){
       vkFreeCommandBuffers(cfxDevice.device(deviceIndex), cfxDevice.getCommandPool(deviceIndex), static_cast<uint32_t>(commandBuffers[deviceIndex].size()), commandBuffers[deviceIndex].data());
   }


  
   
    RenderBuffer Renderer::beginFrame(){
        // std::cout << "BEGIN FRAME"<< std::endl;
        RenderBuffer renderBuffer{};
        VkCommandBuffer commandBuffer{};
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        assert(!isFrameStarted && "Cant call beginFrame while frame is in progress");
            isFrameStarted = true;
        
            
        if(cfxDevice.getDevicesinDeviceGroup() > 1 ){
            if(currentImageIndex % 2 == 0){
                deviceIndex = 0;
            }
            else{
                deviceIndex = 1;
            }
            commandBuffer = getCurrentCommandBuffer(deviceIndex);
            renderBuffer.commandBuffer = commandBuffer;
            
            renderBuffer.deviceIndex = deviceIndex;
            
        

        }
        else{
            renderBuffer.deviceIndex = 0;
            renderBuffer.deviceMask = 1;
            deviceIndex = 0;
            commandBuffer = getCurrentCommandBuffer(deviceIndex);
            renderBuffer.commandBuffer = commandBuffer;
            renderBuffer.deviceIndex = deviceIndex;

        }
            auto result = cfxSwapChain->acquireNextImage(&currentImageIndex,deviceIndex);
            if(result == VK_ERROR_OUT_OF_DATE_KHR){
                recreateSwapChain();
                renderBuffer.commandBuffer = nullptr;
                return renderBuffer;
            }

            if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR){
                throw std::runtime_error("failed to aquire swap chain image");
            }

            
       

        
        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
                    throw std::runtime_error("failed to begin recording command buffer!");
          }
        //   if(cfxDevice.getDevicesinDeviceGroup() > 1){
        //       vkCmdSetDeviceMask(commandBuffer,renderBuffer.deviceMask);
        //   }
          
          
          
          
        return renderBuffer;

    }
    void Renderer::endFrame(){
        assert(isFrameStarted && "cant call endFrame while frame is not in progress");
        auto commandBuffer = getCurrentCommandBuffer(deviceIndex);
         
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
    void Renderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer,uint32_t deviceMask,uint32_t deviceIndex){
        assert(isFrameStarted && "Cant call beginSwapChainRenderPass if frame is not in progress");
        assert(commandBuffer == getCurrentCommandBuffer(deviceIndex) && "cant begin renderpass on a command buffer from a different frame");


        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = cfxSwapChain->getRenderPass(deviceIndex);
        renderPassInfo.framebuffer = cfxSwapChain->getFrameBuffer(deviceIndex,currentImageIndex);
          // renderPassInfo.pNext = &deviceGroupRenderPassInfo;

        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = cfxSwapChain->getSwapChainExtent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();
        // if(cfxDevice.getDevicesinDeviceGroup() > 1){
        //       vkCmdSetDeviceMask(commandBuffer,deviceMask);
        //   }
        

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
    void Renderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer,uint32_t deviceMask,int deviceIndex){
        assert(isFrameStarted && "Cant call endSwapChainRenderPass if frame is not in progress");
        assert(commandBuffer == getCurrentCommandBuffer(deviceIndex) && "cant end renderpass on a command buffer from a different frame");
        // if(cfxDevice.getDevicesinDeviceGroup() > 1){
        //       vkCmdSetDeviceMask(commandBuffer,deviceMask);
        //   }
        vkCmdEndRenderPass(commandBuffer);
        //   std::cout << "END RENDER PASS" << std::endl;

    }
    
  
  
}