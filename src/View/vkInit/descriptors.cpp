#include "View/vkInit/descriptors.h"

void vkInit::make_descriptor_set_layout(VkDevice device, const descriptorSetLayoutData& bindings, VkDescriptorSetLayout descriptorSetLayout) {

	/*
		 Bindings describes a whole bunch of descriptor types, collect them all into a
		 list of some kind.
	 */
	std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
	layoutBindings.reserve(bindings.count);

	for (int i = 0; i < bindings.count; i++) {

		/*
			typedef struct VkDescriptorSetLayoutBinding {
				uint32_t              binding;
				VkDescriptorType      descriptorType;
				uint32_t              descriptorCount;
				VkShaderStageFlags    stageFlags;
				const VkSampler*      pImmutableSamplers;
			} VkDescriptorSetLayoutBinding;
		*/

		VkDescriptorSetLayoutBinding layoutBinding;
		layoutBinding.binding = bindings.indices[i];
		layoutBinding.descriptorType = bindings.types[i];
		layoutBinding.descriptorCount = bindings.counts[i];
		layoutBinding.stageFlags = bindings.stages[i];
		layoutBindings.push_back(layoutBinding);
	}

	/*
		typedef struct VkDescriptorSetLayoutCreateInfo {
			VkStructureType                        sType;
			const void*                            pNext;
			VkDescriptorSetLayoutCreateFlags       flags;
			uint32_t                               bindingCount;
			const VkDescriptorSetLayoutBinding*    pBindings;
		} VkDescriptorSetLayoutCreateInfo;
	*/
	VkDescriptorSetLayoutCreateInfo layoutInfo;
	layoutInfo.flags = VkDescriptorSetLayoutCreateFlagBits();
	layoutInfo.bindingCount = bindings.count;
	layoutInfo.pBindings = layoutBindings.data(); 
	VkResult result = vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout);

	if (result != VK_SUCCESS) {
		std::cout << "Failed to create Descriptor Set Layout, error code: " << result << std::endl;
		
	}

	
}

void vkInit::make_descriptor_pool(VkDevice device, VkDescriptorPool& descriptorPool,uint32_t size, const descriptorSetLayoutData& bindings)  {
	

		std::vector<VkDescriptorPoolSize> poolSizes;
		/*
			typedef struct VkDescriptorPoolSize {
				VkDescriptorType    type;
				uint32_t            descriptorCount;
			} VkDescriptorPoolSize;
		*/

		for (int i = 0; i < bindings.count; i++) {

			VkDescriptorPoolSize poolSize;
			poolSize.type = bindings.types[i];
			poolSize.descriptorCount = size;
			poolSizes.push_back(poolSize);
		}

		VkDescriptorPoolCreateInfo poolInfo;
		/*
			typedef struct VkDescriptorPoolCreateInfo {
				VkStructureType                sType;
				const void*                    pNext;
				VkDescriptorPoolCreateFlags    flags;
				uint32_t                       maxSets;
				uint32_t                       poolSizeCount;
				const VkDescriptorPoolSize*    pPoolSizes;
			} VkDescriptorPoolCreateInfo;
		*/

		poolInfo.flags = VkDescriptorPoolCreateFlags();
		poolInfo.maxSets = size;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();

		VkResult result = vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool);
		if (result != VK_SUCCESS) {
			std::cout << "Failed to make descriptor pool, error code: " << result << std::endl;
			
		}
}

void vkInit::make_imgui_descriptor_pool(VkDevice device, VkDescriptorPool descriptorPool)
{
	VkDescriptorPoolSize poolSizes[] = {
		{ VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 }
	};
	VkDescriptorPoolCreateInfo poolInfo;
	/*
		typedef struct VkDescriptorPoolCreateInfo {
			VkStructureType                sType;
			const void*                    pNext;
			VkDescriptorPoolCreateFlags    flags;
			uint32_t                       maxSets;
			uint32_t                       poolSizeCount;
			const VkDescriptorPoolSize*    pPoolSizes;
		} VkDescriptorPoolCreateInfo;
	*/

	poolInfo.flags = VkDescriptorPoolCreateFlags();
	poolInfo.maxSets = 1000;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = poolSizes;

	VkResult result = vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool);

	if (result != VK_SUCCESS) {
		std::cout << "Failed to make descriptor pool, error code: " << result << std::endl;
		
	}
}

void vkInit::allocate_descriptor_set(VkDevice device, VkDescriptorSet& descriptorSet,VkDescriptorPool descriptorPool, VkDescriptorSetLayout layout)
{
	VkDescriptorSetAllocateInfo allocationInfo;
	/*
		typedef struct VkDescriptorSetAllocateInfo {
			VkStructureType                 sType;
			const void*                     pNext;
			VkDescriptorPool                descriptorPool;
			uint32_t                        descriptorSetCount;
			const VkDescriptorSetLayout*    pSetLayouts;
		} VkDescriptorSetAllocateInfo;
	*/

	allocationInfo.descriptorPool = descriptorPool;
	allocationInfo.descriptorSetCount = 1;
	allocationInfo.pSetLayouts = &layout;

	VkResult result = vkAllocateDescriptorSets(device, &allocationInfo, &descriptorSet);
	if (result != VK_SUCCESS) {
		std::cout << "Failed to allocate descriptor set from pool, error code: " << result << std::endl;
		
	}
	
}
