#pragma once
#include "config.h"
#include "View/vkUtil/Pipelines/pipelineStructs.h"
#include <vector>
namespace vkInit {


	class ComputePipelineBuilder {


		VkPipelineShaderStageCreateInfo computeShaderInfo;
		VkShaderModule computeShader = nullptr;
		VkDevice device;
		VkPushConstantRange pushConstantInfo;
		int pushConstantCount = 0;
		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
		void reset();
		

	public:
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
		ComputePipelineBuilder(VkDevice device);
		~ComputePipelineBuilder();
		vkUtil::GraphicsPipelineOutBundle build(bool debugMode);
		void make_pipeline_layout(VkPipelineLayout& pipelineLayout);
		void specify_compute_shader(const char* filename);
		void set_push_constant(VkShaderStageFlags stage, size_t size, int count);
		void add_descriptor_set_layout(VkDescriptorSetLayout descriptorSetLayout);
		VkPipelineShaderStageCreateInfo make_shader_info(
			const VkShaderModule& shaderModule, const VkShaderStageFlagBits& stage);


		void reset_descriptor_set_layouts();
		void reset_shader_modules();
	};

}