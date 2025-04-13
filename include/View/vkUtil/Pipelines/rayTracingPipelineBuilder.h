#pragma once
#include "config.h"
#include "View/vkUtil/Pipelines/pipelineStructs.h"
#include <vector>
#include <unordered_map>

namespace vkInit {
	

	uint32_t alignedSize(uint32_t value, uint32_t alignment);

	class RayTracingPipelineBuilder {
	
	
	private:
		VkDevice device;
		VkPhysicalDevice physicalDevice;
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts{};
		std::vector<VkPipelineShaderStageCreateInfo> shaderStages{};
		std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups{};

		VkPushConstantRange pushConstantInfo;
		int pushConstantCount = 0;
		VkShaderModule rayGenShader = nullptr, allHitShader = nullptr , closestHitShader = nullptr;
		std::vector<VkShaderModule> missShaders;
		VkPipelineShaderStageCreateInfo rayGenShaderInfo, missShaderInfo, allHitShaderInfo, closestHitShaderInfo;
		VkPipelineShaderStageCreateInfo make_shader_info(
			const VkShaderModule& shaderModule, const VkShaderStageFlagBits& stage);
		
		void create_shader_groups(VkPipeline pipeline, VkQueue queue, VkCommandBuffer commandBuffer, Buffer& raygenShaderBindingTable, Buffer& missShaderBindingTable, Buffer& hitShaderBindingTable);
		
		
	public:
		uint32_t recursive;
		VkSpecializationInfo specializationInfo = {};
		VkSpecializationMapEntry specializationMapEntry = {};


		void* gemMemoryLocation = nullptr;
		void* missMemoryLocation = nullptr;
		void* hitMemoryLocation = nullptr;
		RayTracingPipelineBuilder(VkPhysicalDevice physicalDevice,VkDevice device);
		~RayTracingPipelineBuilder();
		void make_pipeline_layout(VkPipelineLayout& pipelineLayout);
		void specify_ray_gen_shader(const char* filename, uint32_t maxRecursive);
		void specify_miss_shader(const char* filename);
		void specify_closest_hit_shader(const char* filename);
		void specify_all_hit_shader(const char* filename);
		vkUtil::GraphicsPipelineOutBundle build(VkQueue queue, VkCommandBuffer commandBuffer, Buffer& raygenShaderBindingTable, Buffer& missShaderBindingTable, Buffer& hitShaderBindingTable);
		void add_descriptor_set_layout(VkDescriptorSetLayout descriptorSetLayout);
		

	};
}