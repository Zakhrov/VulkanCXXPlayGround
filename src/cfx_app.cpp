#include "cfx_app.hpp"
#include<stdexcept>
#include<array>


namespace cfx{
    App::App(){
        createPipelineLayout();
        createPipeline();
        createCommandBuffers();
    }
    App::~App(){
        vkDestroyPipelineLayout(cfxDevice.device(),pipelineLayout,nullptr);
    }
    void App::run(){
        while(!window.shouldClose()){
            drawFrame();
            glfwPollEvents();
        }

        vkDeviceWaitIdle(cfxDevice.device());
    }
    void App::createPipelineLayout(){
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
        auto pipelineConfig = CFXPipeLine::defaultPipelineConfigInfo(cfxSwapChain.width(),cfxSwapChain.height());
        pipelineConfig.renderPass = cfxSwapChain.getRenderPass();
        pipelineConfig.pipelineLayout = pipelineLayout;
        cfxPipeLine = std::make_unique<CFXPipeLine>(cfxDevice,pipelineConfig,
        "shaders/simple_shader.vert.spv",
        "shaders/simple_shader.frag.spv");
    }

     void App::createCommandBuffers(){
         commandBuffers.resize(cfxSwapChain.imageCount());
         VkCommandBufferAllocateInfo allocInfo{};
         allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
         allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
         allocInfo.commandPool = cfxDevice.getCommandPool();
         allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

         if(vkAllocateCommandBuffers(cfxDevice.device(),&allocInfo,commandBuffers.data()) != VK_SUCCESS){
             throw std::runtime_error("failed to allocate command buffers");
         }
         for(int i=0; i<commandBuffers.size(); i++){
             VkCommandBufferBeginInfo beginInfo{};
             beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

             if(vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS){
                 throw std::runtime_error("failed to begin command buffer");
             }

             VkRenderPassBeginInfo renderPassInfo{};
             renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
             renderPassInfo.renderPass = cfxSwapChain.getRenderPass();
             renderPassInfo.framebuffer = cfxSwapChain.getFrameBuffer(i);
             renderPassInfo.renderArea.offset = {0,0};
             renderPassInfo.renderArea.extent = cfxSwapChain.getSwapChainExtent();

             std::array<VkClearValue, 2> clearValues{};
             clearValues[0].color = {0.1f,0.1f,0.1f,1.0f};
             clearValues[1].depthStencil = {1.0f,0};
             renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
             renderPassInfo.pClearValues = clearValues.data();

             vkCmdBeginRenderPass(commandBuffers[i],&renderPassInfo,VK_SUBPASS_CONTENTS_INLINE);

             cfxPipeLine->bind(commandBuffers[i]);
             vkCmdDraw(commandBuffers[i],3,1,0,0);

             vkCmdEndRenderPass(commandBuffers[i]);
             if(vkEndCommandBuffer(commandBuffers[i])!=VK_SUCCESS){
                 throw std::runtime_error("failed to record command buffer");
             }





         }


     }
        void App::drawFrame(){
            uint32_t imageIndex;
            auto result = cfxSwapChain.acquireNextImage(&imageIndex);

            if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR){
                throw std::runtime_error("failed to aquire swap chain image");
            }

            result = cfxSwapChain.submitCommandBuffers(&commandBuffers[imageIndex],&imageIndex);
            if(result != VK_SUCCESS){
                throw std::runtime_error("failed to present swap chain image");
            }

        }
}