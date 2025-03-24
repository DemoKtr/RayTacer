#pragma once
#include "config.h"
#include "View/vkUtil/Pipelines/pipelineStructs.h"
#include <vector>
#include <unordered_map>

namespace vkInit {
	
	class RayTracingPipelineBuilder {
	
	
	private:
		VkDevice device;
		VkPhysicalDevice physicalDevice;
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts{};
		std::vector<VkPipelineShaderStageCreateInfo> shaderStages{};
		std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups{};

		VkPushConstantRange pushConstantInfo;
		int pushConstantCount = 0;
		VkShaderModule rayGenShader = nullptr, missShader = nullptr, allHitShader = nullptr , closestHitShader = nullptr;
		VkPipelineShaderStageCreateInfo rayGenShaderInfo, missShaderInfo, allHitShaderInfo, closestHitShaderInfo;
		VkPipelineShaderStageCreateInfo make_shader_info(
			const VkShaderModule& shaderModule, const VkShaderStageFlagBits& stage);
		
		void create_shader_groups();
		
	public:
		Buffer raygenShaderBindingTable;
		Buffer missShaderBindingTable;
		Buffer hitShaderBindingTable;
		RayTracingPipelineBuilder(VkPhysicalDevice physicalDevice,VkDevice device);
		~RayTracingPipelineBuilder();
		VkPipelineLayout make_pipeline_layout(VkPipelineLayout& pipelineLayout);
		void specify_ray_gen_shader(const char* filename);
		void specify_miss_shader(const char* filename);
		void specify_closest_hit_shader(const char* filename);
		void specify_all_hit_shader(const char* filename);
		vkUtil::GraphicsPipelineOutBundle build();
		void add_descriptor_set_layout(VkDescriptorSetLayout descriptorSetLayout);
		

	};
}