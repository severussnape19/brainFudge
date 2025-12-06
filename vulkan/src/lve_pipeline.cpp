#include "lve_pipeline.hpp"

#include <fstream>
#include <stdexcept>
#include <iostream>
#include <cassert>

namespace lve {

    LvePipeline::LvePipeline(
                LveDevice &device, 
                const std::string& vertFilepath, 
                const std::string& fragFilepath, 
                const PipelineConfigInfo &configInfo) : lveDevice(device) {
        createGraphicsPipeline(vertFilepath, fragFilepath, configInfo);
    }

    LvePipeline::~LvePipeline() {
        vkDestroyShaderModule(lveDevice.device(), vertexShaderModule, nullptr);
        vkDestroyShaderModule(lveDevice.device(), fragShaderModule, nullptr);
        vkDestroyPipeline(lveDevice.device(), graphicsPipeLine, nullptr);
    }
    
    // just reads the glsg files as a binary instead of text
    std::vector<char> LvePipeline::readFile(const std::string& filepath) {
        std::ifstream file{filepath, std::ios::ate | std::ios::binary}; //std::ios::ate and std::ios::binary are bit flags
        
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file : " + filepath);
        }

        size_t filesize = static_cast<size_t>(file.tellg());
        std::vector<char> buffer(filesize);

        file.seekg(0); // goes to the start of the file
        file.read(buffer.data(), filesize); // reads the file

        file.close();
        return buffer;
    };

    void LvePipeline::createGraphicsPipeline(
        const std::string& vertFilepath, const std::string& fragFilepath, const PipelineConfigInfo& configInfo) {

            // Reads shader files from disk
            auto vertCode = readFile(vertFilepath);
            auto fragCode = readFile(fragFilepath);

            assert(configInfo.pipelineLayout != VK_NULL_HANDLE && 
                "cannot create graphics pipeline:: no pipelineLayout provided in configinfo");

            assert(configInfo.renderPass != VK_NULL_HANDLE && 
                "cannot create graphics pipeline:: no pipelineLayout provided in configinfo");
            // Creates vulkan shader modules
            // Shaders on disk are useless until you wrap them into VKshaderModule objects
            // These shader modules:
            // get owned by the GPU
            // Stays alive aslong as the piepline needs them
            // must be destroyed after pipeline creation is finished
            // We are basically giving the bytecode to vulkan and asking it to prepare it
            createShaderModule(vertCode, &vertexShaderModule);
            createShaderModule(fragCode, &fragShaderModule);

/*
            # This array holds two shader stages
             -> vertex shader
             -> fragment shader
            # Now we populate them
*/          VkPipelineShaderStageCreateInfo shaderStages[2];
            shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
            shaderStages[0].module = vertexShaderModule;
            shaderStages[0].pName = "main";
            shaderStages[0].flags = 0;
            shaderStages[0].pNext = nullptr;
            shaderStages[0].pSpecializationInfo = nullptr;

            shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            shaderStages[1].module = fragShaderModule;
            shaderStages[1].pName = "main";
            shaderStages[1].flags = 0;
            shaderStages[1].pNext = nullptr;
            shaderStages[1].pSpecializationInfo = nullptr;

            // This struct is used to describe how we interpret our input vertex buffer data that is the initial input to the graphics pipeline
            VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
            vertexInputInfo.vertexAttributeDescriptionCount = 0;
            vertexInputInfo.vertexBindingDescriptionCount = 0;
            vertexInputInfo.pVertexAttributeDescriptions = nullptr;
            vertexInputInfo.pVertexBindingDescriptions = nullptr;

            // This struct will use all the above config we went through to create our actual graphics pipeline object
            VkGraphicsPipelineCreateInfo pipelineInfo{};
            pipelineInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            pipelineInfo.stageCount = 2; // stage count specifies how many programmable stages our pipeline will use. (we have vertex and fragment shaders for now)
            pipelineInfo.pStages = shaderStages;
            pipelineInfo.pVertexInputState = &vertexInputInfo;
            pipelineInfo.pInputAssemblyState = &configInfo.inputAssemblyInfo;
            pipelineInfo.pViewportState = &configInfo.viewportInfo;
            pipelineInfo.pRasterizationState= &configInfo.rasterizationInfo;
            pipelineInfo.pMultisampleState = &configInfo.multisampleInfo;
            pipelineInfo.pColorBlendState = &configInfo.colorBlendInfo;
            pipelineInfo.pDepthStencilState= &configInfo.depthStencilInfo;
            pipelineInfo.pDynamicState = nullptr;

            pipelineInfo.layout = configInfo.pipelineLayout;
            pipelineInfo.renderPass = configInfo.renderPass;
            pipelineInfo.subpass = configInfo.subpass;

            // These can be used to optimize performance
            pipelineInfo.basePipelineIndex = -1;
            pipelineInfo.basePipelineHandle =  VK_NULL_HANDLE;

            if (vkCreateGraphicsPipelines(lveDevice.device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeLine) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create a graphics pipeline");
            }
    }

    void LvePipeline::createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule) {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        if (vkCreateShaderModule(lveDevice.device(), &createInfo, nullptr, shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create a shader module");
        }
    }

    PipelineConfigInfo LvePipeline::defaultPipelineConfigInfo(uint32_t width, uint32_t height) {
        PipelineConfigInfo configInfo{};

        // this is the first stage of the pipeline and takes input a group of vertices and groups them into geometry
        // Input Assembly
        // We are telling vulkan how to group your vertices
        configInfo.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        configInfo.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // says every group of three vertices is a triangle
        configInfo.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;
        
        // Viewport + scissor
        // viewport = "transform normalized device coordinates into pixel coords"
        // scissor = "only draw inside this rectangle; clip everything outside"
        configInfo.viewport.x = 0.0f;
        configInfo.viewport.y = 0.0f;
        configInfo.viewport.width = static_cast<float>(width);
        configInfo.viewport.height = static_cast<float>(height);
        configInfo.viewport.minDepth = 0.0f;
        configInfo.viewport.maxDepth = 1.0f;
        // scissor
        configInfo.scissor.offset = {0, 0};
        configInfo.scissor.extent = {width, height};
        
        // ViewPort state
        // We tell vulkan we have exactly one viewport and one scissor
        configInfo.viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        configInfo.viewportInfo.viewportCount = 1;
        configInfo.viewportInfo.pViewports = &configInfo.viewport;
        configInfo.viewportInfo.scissorCount = 1;
        configInfo.viewportInfo.pScissors = &configInfo.scissor;
        
        // Rasterize
        configInfo.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        configInfo.rasterizationInfo.depthClampEnable = VK_FALSE;
        configInfo.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
        configInfo.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;  // render solid triangles(not wireframe)
        configInfo.rasterizationInfo.lineWidth = 1.0f;
        configInfo.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;     // Draw both sides
        configInfo.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE; // which winding counts as front (direct in which the triangle is formed )
        configInfo.rasterizationInfo.depthBiasEnable = VK_FALSE;
        configInfo.rasterizationInfo.depthBiasConstantFactor = 0.0f;  // Optional
        configInfo.rasterizationInfo.depthBiasClamp = 0.0f;           // Optional
        configInfo.rasterizationInfo.depthBiasSlopeFactor = 0.0f;     // Optional
        
        // This controls anti-aliasing
        configInfo.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        configInfo.multisampleInfo.sampleShadingEnable = VK_FALSE;
        configInfo.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        configInfo.multisampleInfo.minSampleShading = 1.0f;           // Optional
        configInfo.multisampleInfo.pSampleMask = nullptr;             // Optional
        configInfo.multisampleInfo.alphaToCoverageEnable = VK_FALSE;  // Optional
        configInfo.multisampleInfo.alphaToOneEnable = VK_FALSE;       // Optional
        
        // Color blending = how new colors mix with old colors in the framebuffer
        configInfo.colorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT;
        configInfo.colorBlendAttachment.blendEnable = VK_FALSE;
        configInfo.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
        configInfo.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
        configInfo.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;              // Optional
        configInfo.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
        configInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
        configInfo.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;              // Optional
        
        // color blend state
        configInfo.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        configInfo.colorBlendInfo.logicOpEnable = VK_FALSE;
        configInfo.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;  // Optional
        configInfo.colorBlendInfo.attachmentCount = 1;
        configInfo.colorBlendInfo.pAttachments = &configInfo.colorBlendAttachment;
        configInfo.colorBlendInfo.blendConstants[0] = 0.0f;  // Optional
        configInfo.colorBlendInfo.blendConstants[1] = 0.0f;  // Optional
        configInfo.colorBlendInfo.blendConstants[2] = 0.0f;  // Optional
        configInfo.colorBlendInfo.blendConstants[3] = 0.0f;  // Optional
        
        // Depth and stencil
        // controls visibility
        configInfo.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        configInfo.depthStencilInfo.depthTestEnable = VK_TRUE;   // dont draw things behind other things
        configInfo.depthStencilInfo.depthWriteEnable = VK_TRUE;  // write to depth buffer
        configInfo.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS; // closer fragments win
        configInfo.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
        configInfo.depthStencilInfo.minDepthBounds = 0.0f;  // Optional
        configInfo.depthStencilInfo.maxDepthBounds = 1.0f;  // Optional
        configInfo.depthStencilInfo.stencilTestEnable = VK_FALSE;
        configInfo.depthStencilInfo.front = {};  // Optional
        configInfo.depthStencilInfo.back = {};   // Optional

        return configInfo;
    }

}
                                                    