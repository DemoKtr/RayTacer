#include "View/vkInit/synchronizer.h"

void vkInit::make_semaphore(VkDevice device, VkSemaphore& semaphore, bool debug) {
    VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.flags = VkSemaphoreCreateFlags();
	semaphoreInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	VkResult result = vkCreateSemaphore(device, &semaphoreInfo, nullptr, &semaphore);

	if (result != VK_SUCCESS) {
		if (debug) {
			std::cout << "Failed to create semaphore!" << std::endl;
		}
		
	}
}

void vkInit::make_fence(VkDevice device, VkFence& fence,bool debug) {
	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.flags = VkFenceCreateFlags() | VkFenceCreateFlagBits::VK_FENCE_CREATE_SIGNALED_BIT;
	fenceInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	VkResult result = vkCreateFence(device, &fenceInfo, nullptr, &fence);

	if (result != VK_SUCCESS) {
		if (debug) {
			std::cout << "Failed to create fence!" << std::endl;
		}
		
	}

	
}
