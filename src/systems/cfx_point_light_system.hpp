#pragma once


#include "../cfx_pipeline.hpp"
#include "../cfx_device.hpp"
#include "../cfx_camera.hpp"

#include "../cfx_model.hpp"
#include "../cfx_game_object.hpp"
#include "../cfx_frame_info.hpp"
#include "../cfx_descriptors.hpp"
#include<memory>
#include<vector>
#include <glm/glm.hpp>
namespace cfx{
    class CFXPointLightSystem{
        public:
        CFXPointLightSystem(CFXDevice &device, std::vector<VkRenderPass> renderPasses,std::vector<std::unique_ptr<CFXDescriptorSetLayout>> &cfxSetLayouts);
        ~CFXPointLightSystem();
        CFXPointLightSystem(const CFXPointLightSystem &) = delete;
        CFXPointLightSystem &operator=(const CFXPointLightSystem &) = delete;
        void render(FrameInfo &frameInfo);
        


        private:
        
        void createPipelineLayout(VkDescriptorSetLayout descriptorSetLayout,int deviceIndex);
        void createPipeline(VkRenderPass renderpass,int deviceIndex);
        
        
        
        
        
    

        
        CFXDevice& cfxDevice;
        // CFXSwapChain cfxSwapChain{cfxDevice,window.getExtent()};
        
        std::vector<std::unique_ptr<CFXPipeLine>> cfxPipeLines;
        std::vector<VkPipelineLayout> pipelineLayout;
        
        
        
    };
}