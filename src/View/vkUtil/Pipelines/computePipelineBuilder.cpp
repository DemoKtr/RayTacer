#include "View/vkUtil/Pipelines/computePipelineBuilder.h"
#include <View/vkUtil/Pipelines/shaders.h>



vkInit::ComputePipelineBuilder::ComputePipelineBuilder(VkDevice device){
    this->device = device;
    reset();

   
}

vkInit::ComputePipelineBuilder::~ComputePipelineBuilder() {
    reset();
}

void vkInit::ComputePipelineBuilder::reset() {
    reset_shader_modules();
    reset_descriptor_set_layouts();

    pushConstantCount = 0;
    pushConstantInfo.offset = 0;
    pushConstantInfo.size = 0;
}

void vkInit::ComputePipelineBuilder::reset_descriptor_set_layouts() {
    descriptorSetLayouts.clear();
}

void vkInit::ComputePipelineBuilder::reset_shader_modules() {
    if (computeShader) {
        vkDestroyShaderModule(device, computeShader, nullptr);
    }
    shaderStages.clear();
}

vkUtil::GraphicsPipelineOutBundle vkInit::ComputePipelineBuilder::build(bool debugMode) {
    vkUtil::GraphicsPipelineOutBundle output;
    VkPipelineLayout ComputePipelineLayout; 
    make_pipeline_layout(ComputePipelineLayout);
    VkComputePipelineCreateInfo computePipelineCreateInfo = {};
    computePipelineCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    computePipelineCreateInfo.layout = ComputePipelineLayout;
    computePipelineCreateInfo.flags = {};
    computePipelineCreateInfo.stage = computeShaderInfo;
    
    
    VkPipeline computePipeline = {};
    if (debugMode) std::cout << "Creating compute particle Pipeline " << std::endl;
    VkResult result = vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, &computePipeline);

    if (result != VK_SUCCESS) {
        if (debugMode) std::cerr << "Failed to create particle Compute Pipeline! Error code: " << result << std::endl;
        return vkUtil::GraphicsPipelineOutBundle();
        
    }
    output.pipeline = computePipeline;
    output.layout = ComputePipelineLayout;
    return output;
}

void vkInit::ComputePipelineBuilder::make_pipeline_layout(VkPipelineLayout& pipelineLayout) {

    /*
    typedef struct VkPipelineLayoutCreateInfo {
        VkStructureType                 sType;
        const void*                     pNext;
        VkPipelineLayoutCreateFlags     flags;
        uint32_t                        setLayoutCount;
        const VkDescriptorSetLayout*    pSetLayouts;
        uint32_t                        pushConstantRangeCount;
        const VkPushConstantRange*      pPushConstantRanges;
    } VkPipelineLayoutCreateInfo;
    */

    VkPipelineLayoutCreateInfo layoutInfo;
    layoutInfo.flags = VkPipelineLayoutCreateFlags();

    layoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
    layoutInfo.pSetLayouts = descriptorSetLayouts.data();



    layoutInfo.pushConstantRangeCount = pushConstantCount;
    layoutInfo.pPushConstantRanges = &pushConstantInfo;




    VkResult result = vkCreatePipelineLayout(device, &layoutInfo, nullptr, &pipelineLayout);

    if (result != VK_SUCCESS) {
        std::cerr << "Failed to create pipeline layout! Error code: " << result << std::endl;
    }
}

void vkInit::ComputePipelineBuilder::specify_compute_shader(const char* filename) {
    if (computeShader) {
        vkDestroyShaderModule(device, computeShader, nullptr);
        computeShader = nullptr;
    }

    std::cout << "Create compute shader module" << std::endl;
    vkUtil::createModule(filename, device, computeShader);
    computeShaderInfo = make_shader_info(computeShader, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT);
    shaderStages.push_back(computeShaderInfo);
}

void vkInit::ComputePipelineBuilder::set_push_constant(VkShaderStageFlags stage, size_t size, int count) {

    pushConstantCount = count;
    pushConstantInfo.offset = 0;
    pushConstantInfo.size = size;
    pushConstantInfo.stageFlags = stage;
}

void vkInit::ComputePipelineBuilder::add_descriptor_set_layout(VkDescriptorSetLayout descriptorSetLayout) {
    descriptorSetLayouts.push_back(descriptorSetLayout);
}

VkPipelineShaderStageCreateInfo vkInit::ComputePipelineBuilder::make_shader_info(const VkShaderModule& shaderModule, const VkShaderStageFlagBits& stage) {

    VkPipelineShaderStageCreateInfo shaderInfo = {};
    shaderInfo.flags = VkPipelineShaderStageCreateFlags();
    shaderInfo.stage = stage;
    shaderInfo.module = shaderModule;
    shaderInfo.pName = "main";
    return shaderInfo;
}
