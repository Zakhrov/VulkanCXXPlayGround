#include "cfx_app.hpp"
#include<stdexcept>
#include<array>
#include<iostream>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>


namespace cfx{
  struct SimplePushConstantData{
    glm::mat2 transform{1.f};
    glm::vec2 offset;
    alignas(16) glm::vec3 color;

  };
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
      VkPushConstantRange pushConstantRange{};
      pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
      pushConstantRange.offset = 0;
      pushConstantRange.size = sizeof(SimplePushConstantData);
        std::cout << "CREATE PIPELINE LAYOUT"<< std::endl;
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        pipelineLayoutInfo.pushConstantRangeCount = 1;

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

      commandBuffers.resize(cfxSwapChain->imageCount());

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

     void App::recordCommandBuffer(int frame,int imageIndex,uint32_t deviceIndex){
       static int currentFrame = 0;
       currentFrame = (currentFrame + 1) % 1000;

        VkDeviceGroupCommandBufferBeginInfo deviceGroupCommandBufferInfo{};
        deviceGroupCommandBufferInfo.sType = VK_STRUCTURE_TYPE_DEVICE_GROUP_COMMAND_BUFFER_BEGIN_INFO;
        deviceGroupCommandBufferInfo.deviceMask = deviceIndex;
        VkDeviceGroupRenderPassBeginInfo deviceGroupRenderPassInfo{};
             deviceGroupRenderPassInfo.sType = VK_STRUCTURE_TYPE_DEVICE_GROUP_RENDER_PASS_BEGIN_INFO;
             deviceGroupRenderPassInfo.deviceMask = deviceIndex;
             deviceGroupRenderPassInfo.deviceRenderAreaCount = cfxDevice.getDeviceRects().size();
             deviceGroupRenderPassInfo.pDeviceRenderAreas = cfxDevice.getDeviceRects().data();
            
            VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.pNext = &deviceGroupCommandBufferInfo;

  if (vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS) {
    throw std::runtime_error("failed to begin recording command buffer!");
  }

  VkRenderPassBeginInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = cfxSwapChain->getRenderPass();
  renderPassInfo.framebuffer = cfxSwapChain->getFrameBuffer(imageIndex);
  renderPassInfo.pNext = &deviceGroupRenderPassInfo;

  renderPassInfo.renderArea.offset = {0, 0};
  renderPassInfo.renderArea.extent = cfxSwapChain->getSwapChainExtent();

  std::array<VkClearValue, 2> clearValues{};
  clearValues[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
  clearValues[1].depthStencil = {1.0f, 0};
  renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
  renderPassInfo.pClearValues = clearValues.data();

  vkCmdBeginRenderPass(commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = static_cast<float>(cfxSwapChain->getSwapChainExtent().width);
  viewport.height = static_cast<float>(cfxSwapChain->getSwapChainExtent().height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  VkRect2D scissor{{0, 0}, cfxSwapChain->getSwapChainExtent()};
  vkCmdSetViewport(commandBuffers[imageIndex], 0, 1, &viewport);
  vkCmdSetScissor(commandBuffers[imageIndex], 0, 1, &scissor);

  cfxPipeLine->bind(commandBuffers[imageIndex]);
  cfxModel->bind(commandBuffers[imageIndex]);
  for(int j=0; j< 4; j++){
    SimplePushConstantData push{};
    push.offset = {-0.5f + currentFrame * 0.002f,-0.4f + j * 0.25f};
    push.color = {0.0f,0.0f,0.2f + 0.2f * j };
    vkCmdPushConstants(commandBuffers[imageIndex],pipelineLayout,VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT ,0,sizeof(SimplePushConstantData),&push);
    cfxModel->draw(commandBuffers[imageIndex]);
  }
  
  

  vkCmdEndRenderPass(commandBuffers[imageIndex]);
  if (vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS) {
    throw std::runtime_error("failed to record command buffer!");
  }
     }
    void App::drawFrame(int frame){
            // uint32_t imageIndex;
            // auto result = cfxSwapChain->acquireNextImage(&imageIndex);
            // if(result == VK_ERROR_OUT_OF_DATE_KHR){
            //     recreateSwapChain();
            //     return;
            // }

            // if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR){
            //     throw std::runtime_error("failed to aquire swap chain image");
            // }
             
            

            // recordCommandBuffer(frame,imageIndex);
            //     result = cfxSwapChain->submitCommandBuffers(&commandBuffers[imageIndex],&imageIndex);
            // if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.wasWindowResized()){
            //     window.restWindowResizedFlag();
            //     recreateSwapChain();
            //     return;

            // }
            // if(result != VK_SUCCESS){
            //     throw std::runtime_error("failed to present swap chain image");
            // }
          uint32_t imageIndex;
  auto result = cfxSwapChain->acquireNextImage(&imageIndex);

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    recreateSwapChain();
    return;
  }

  if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    throw std::runtime_error("failed to acquire swap chain image!");
  }

  recordCommandBuffer(frame,imageIndex,1);
  recordCommandBuffer(frame,imageIndex,2);
  result = cfxSwapChain->submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
      window.wasWindowResized()) {
    window.restWindowResizedFlag();
    recreateSwapChain();
    return;
  } else if (result != VK_SUCCESS) {
    throw std::runtime_error("failed to present swap chain image!");
  }

     }
}