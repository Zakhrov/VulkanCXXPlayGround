#include "cfx_app.hpp"
#include<stdexcept>
#include<array>
#include<iostream>


namespace cfx{
    App::App(){
        loadModels();
        createPipelineLayout();
        recreateSwapChain();
        createCommandBuffers();
    }
    App::~App(){
        vkDestroyPipelineLayout(cfxDevice.device(),pipelineLayout,nullptr);
    }
    void App::run(){
        int frame = 0;
        while(!window.shouldClose()){
            drawFrame(frame);
            glfwPollEvents();
            frame ++;
        }

        vkDeviceWaitIdle(cfxDevice.device());
    }
    void App::sierpinski(
    std::vector<CFXModel::Vertex> &vertices,
    int depth,
    glm::vec2 left,
    glm::vec2 right,
    glm::vec2 top) {
  if (depth <= 0) {
    vertices.push_back({top});
    vertices.push_back({right});
    vertices.push_back({left});
  } else {
    auto leftTop = 0.5f * (left + top);
    auto rightTop = 0.5f * (right + top);
    auto leftRight = 0.5f * (left + right);
    sierpinski(vertices, depth - 1, left, leftRight, leftTop);
    sierpinski(vertices, depth - 1, leftRight, right, rightTop);
    sierpinski(vertices, depth - 1, leftTop, rightTop, top);
  }
}
    void App::loadModels(){
        std::cout << "LOAD MODELS" << std::endl;
        std::vector<CFXModel::Vertex> vertices {
            {{0.0f,-0.5f},{1.0f,0.0f,0.0f}},
            {{0.5f,0.5f},{.0f,1.0f,0.0f}},
            {{-0.5f,0.5f},{0.0f,0.0f,1.0f}},
        };
        // sierpinski(vertices, 5, {-0.5f, 0.5f}, {0.5f, 0.5f}, {0.0f, -0.5f});
        cfxModel = std::make_unique<CFXModel>(cfxDevice,vertices);
    }
    void App::createPipelineLayout(){
        std::cout << "CREATE PIPELINE LAYOUT"<< std::endl;
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;
        pipelineLayoutInfo.pushConstantRangeCount = 0;

        if(vkCreatePipelineLayout(cfxDevice.device(),&pipelineLayoutInfo,nullptr,&pipelineLayout) != VK_SUCCESS){
            throw std::runtime_error("failed to create pipeline layout");
        }
    }
    void App::createPipeline(){
        std::cout << "CREATE PIPELINE"<< std::endl;
        auto pipelineConfig = CFXPipeLine::defaultPipelineConfigInfo(cfxSwapChain->width(),cfxSwapChain->height());
        pipelineConfig.renderPass = cfxSwapChain->getRenderPass();
        pipelineConfig.pipelineLayout = pipelineLayout;
        cfxPipeLine = std::make_unique<CFXPipeLine>(cfxDevice,pipelineConfig,
        "shaders/simple_shader.vert.spv",
        "shaders/simple_shader.frag.spv");
    }
    void App::recreateSwapChain(){
        std::cout << "CREATE SWAPCHAIN"<< std::endl;
        auto extent = window.getExtent();
        while(extent.width == 0 || extent.height == 0){
            extent = window.getExtent();
            glfwWaitEvents();
        }
        vkDeviceWaitIdle(cfxDevice.device());
        cfxSwapChain = std::make_unique<CFXSwapChain>(cfxDevice,extent);
        createPipeline();
    }

     void App::createCommandBuffers(){
         commandBuffers.resize(cfxSwapChain->imageCount());
         VkCommandBufferAllocateInfo allocInfo{};
         allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
         allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
         allocInfo.commandPool = cfxDevice.getCommandPool().front();
         allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

         if(vkAllocateCommandBuffers(cfxDevice.device(),&allocInfo,commandBuffers.data()) != VK_SUCCESS){
             throw std::runtime_error("failed to allocate command buffers");
         }
        


     }
     void App::recordCommandBuffer(int frame,int imageIndex){

         uint32_t deviceMask;
         if(frame %2 ==0){
             deviceMask = 1;

         }
         else{
             deviceMask =2;
         }

        
        
        
        
        VkDeviceGroupCommandBufferBeginInfoKHR deviceGroupCommandBufferInfo{};
        deviceGroupCommandBufferInfo.sType = VK_STRUCTURE_TYPE_DEVICE_GROUP_COMMAND_BUFFER_BEGIN_INFO_KHR;
        deviceGroupCommandBufferInfo.deviceMask = deviceMask;
            
           VkCommandBufferBeginInfo beginInfo{};
             beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
             beginInfo.pNext = &deviceGroupCommandBufferInfo;

             if(vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS){
                 throw std::runtime_error("failed to begin command buffer");
             }
             vkCmdSetDeviceMask(commandBuffers[imageIndex],deviceMask);

             VkRenderPassBeginInfo renderPassInfo{};
             VkDeviceGroupRenderPassBeginInfoKHR deviceGroupRenderPassInfo{};
             deviceGroupRenderPassInfo.sType = VK_STRUCTURE_TYPE_DEVICE_GROUP_RENDER_PASS_BEGIN_INFO_KHR;
             deviceGroupRenderPassInfo.deviceMask = deviceMask;
             deviceGroupRenderPassInfo.deviceRenderAreaCount = cfxDevice.getDeviceRects().size();
             deviceGroupRenderPassInfo.pDeviceRenderAreas = cfxDevice.getDeviceRects().data();
             renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
             renderPassInfo.renderPass = cfxSwapChain->getRenderPass();
             renderPassInfo.framebuffer = cfxSwapChain->getFrameBuffer(imageIndex);
            //  renderPassInfo.renderArea.offset = {0,0};
            //  renderPassInfo.renderArea.extent = cfxSwapChain->getSwapChainExtent();
             renderPassInfo.pNext = &deviceGroupRenderPassInfo;
             std::cout << "DEVICE RECTS " << cfxDevice.getDeviceRects().size() << std::endl;


             std::array<VkClearValue, 2> clearValues{};
             clearValues[0].color = {0.1f,0.1f,0.1f,1.0f};
             clearValues[1].depthStencil = {1.0f,0};
             renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
             renderPassInfo.pClearValues = clearValues.data();

             vkCmdBeginRenderPass(commandBuffers[imageIndex],&renderPassInfo,VK_SUBPASS_CONTENTS_INLINE);
             

             cfxPipeLine->bind(commandBuffers[imageIndex]);
            cfxModel->bind(commandBuffers[imageIndex]);
            cfxModel->draw(commandBuffers[imageIndex]);

             vkCmdEndRenderPass(commandBuffers[imageIndex]);
             if(vkEndCommandBuffer(commandBuffers[imageIndex])!=VK_SUCCESS){
                 throw std::runtime_error("failed to record command buffer");
             }
     }
        void App::drawFrame(int frame){
            uint32_t imageIndex;
            auto result = cfxSwapChain->acquireNextImage(&imageIndex);
            if(result == VK_ERROR_OUT_OF_DATE_KHR){
                recreateSwapChain();
                return;
            }

            if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR){
                throw std::runtime_error("failed to aquire swap chain image");
            }

            recordCommandBuffer(frame,imageIndex);

            result = cfxSwapChain->submitCommandBuffers(&commandBuffers[imageIndex],&imageIndex);
            if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.wasWindowResized()){
                window.restWindowResizedFlag();
                recreateSwapChain();
                return;

            }
            if(result != VK_SUCCESS){
                throw std::runtime_error("failed to present swap chain image");
            }

        }
}