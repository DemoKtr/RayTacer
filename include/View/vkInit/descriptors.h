#pragma once
#include "config.h"
#include <vector>


namespace vkInit
{

	struct descriptorSetLayoutData {
		int count;
		std::vector<int> indices;
		std::vector<VkDescriptorType> types;
		std::vector<int> counts;
		std::vector<VkShaderStageFlags> stages;
	};

	/**
		Make a descriptor set layout from the given descriptions

		\param device the logical device
		\param bindings	a struct describing the bindings used in the shader
		\returns the created descriptor set layout
	*/
	void make_descriptor_set_layout(VkDevice device, const descriptorSetLayoutData& bindings, VkDescriptorSetLayout& descriptorSetLayout);

	/**
		Make a descriptor pool

		\param device the logical device
		\param size the number of descriptor sets to allocate from the pool
		\param bindings	used to get the descriptor types
		\returns the created descriptor pool
	*/
	void make_descriptor_pool(
		VkDevice device, VkDescriptorPool& descriptorPool, uint32_t size, const descriptorSetLayoutData& bindings);

	void make_imgui_descriptor_pool(VkDevice device, VkDescriptorPool descriptorPool);
	/**
		Allocate a descriptor set from a pool.

		\param device the logical device
		\param descriptorPool the pool to allocate from
		\param layout the descriptor set layout which the set must adhere to
		\returns the allocated descriptor set
	*/
	void allocate_descriptor_set(VkDevice device, VkDescriptorSet& descriptorSet, VkDescriptorPool descriptorPool, VkDescriptorSetLayout layout);


}