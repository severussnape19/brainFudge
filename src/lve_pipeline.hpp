#pragma once

#include "lve_device.hpp"

//std
#include <string>
#include <vector>

namespace lve {

    struct PipelineConfigInfo {
        VkViewport viewport;
        VkRect2D scissor;
        VkPipelineViewportStateCreateInfo viewportInfo;
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
        VkPipelineRasterizationStateCreateInfo rasterizationInfo;
        VkPipelineMultisampleStateCreateInfo multisampleInfo;
        VkPipelineColorBlendAttachmentState colorBlendAttachment;
        VkPipelineColorBlendStateCreateInfo colorBlendInfo;
        VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
        VkPipelineLayout pipelineLayout = nullptr;
        VkRenderPass renderPass = nullptr;
        uint32_t subpass = 0;
        // This is going to contain the data specifying how we want to configure our pipeline
        // The reason we are pulling this information out of our LvePipeline class because we want \n
            // our application layer code to be easily able to configure our pipeline completely as well as share the configuration with other pipelines
    };

    class LvePipeline {
        public:
            LvePipeline(
                LveDevice &device, 
                const std::string& vertFilepath, 
                const std::string& fragFilepath, 
                const PipelineConfigInfo &configInfo);
                
            ~LvePipeline();

            // The below two statements prevent unnecessary copying and accidental copying of pointers to the vulkan objects
            LvePipeline(const LvePipeline&) = delete;
            void operator&(const LvePipeline&) = delete;

            static PipelineConfigInfo defaultPipelineConfigInfo(uint32_t width, uint32_t height);
        
        private:
            static std::vector<char> readFile(const std::string& filepath);

            void createGraphicsPipeline(
                const std::string& vertFilepath, 
                const std::string& fragFilepath,
                const PipelineConfigInfo& confInfo);

            void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

            LveDevice& lveDevice;  // stores the device reference
            VkPipeline graphicsPipeLine;
            VkShaderModule vertexShaderModule;
            VkShaderModule fragShaderModule;
    };
}