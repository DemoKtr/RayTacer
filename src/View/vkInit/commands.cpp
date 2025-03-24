#include "View/vkInit/commands.h"
#include <View/vkUtil/queues.h>

void vkInit::make_command_pool(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool& commandPool,VkSurfaceKHR surface, bool debugMode)
{
	vkUtil::QueueFamilyIndices queueFamilyIndices = vkUtil::findQueueFamilies(physicalDevice, surface, debugMode);
	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.flags = VkCommandPoolCreateFlags() | VkCommandPoolCreateFlagBits::VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
	poolInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    VkResult result = vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool);

    if (result != VK_SUCCESS) {
        if (debugMode)
            std::cout << "Failed Create Main commandPool, error code: " << result << std::endl;   
    }
}

void vkInit::make_compute_command_pool(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool& commandPool, VkSurfaceKHR surface, bool debugMode) {
	vkUtil::QueueFamilyIndices queueFamilyIndices = vkUtil::findQueueFamilies(physicalDevice, surface, debugMode);
	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.flags = VkCommandPoolCreateFlags() | VkCommandPoolCreateFlagBits::VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = queueFamilyIndices.computeFamily.value();
	poolInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

	VkResult result = vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool);
	if (result != VK_SUCCESS) {
		if (debugMode)
			std::cout << "Failed Create Compute commandPool, error code: " << result << std::endl;
	}
	
}

void vkInit::make_transfer_command_pool(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool& commandPool, VkSurfaceKHR surface, bool debugMode) {
	vkUtil::QueueFamilyIndices queueFamilyIndices = vkUtil::findQueueFamilies(physicalDevice, surface, debugMode);
	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.flags = VkCommandPoolCreateFlags() | VkCommandPoolCreateFlagBits::VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = queueFamilyIndices.transferFamily.value();
	poolInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	VkResult result = vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool);
	if (result != VK_SUCCESS) {
		if (debugMode)
			std::cout << "Failed Create Transfer commandPool, error code: " << result << std::endl;
		
	}
	
}

void vkInit::make_command_buffer(commandBufferInputChunk inputChunk, VkCommandBuffer& commandBuffer,bool debugMode) {
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.commandPool = inputChunk.commandPool;
	allocInfo.level = VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;
	allocInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	VkResult result = vkAllocateCommandBuffers(inputChunk.device, &allocInfo, &commandBuffer);
	if (result != VK_SUCCESS) {
		std::cout << "FAILED!!! Allocated main command buffer for frame, error code: " << result << std::endl;
		
	}
	if (debugMode)
		std::cout << "Allocated main command buffer for frame" << std::endl;
	
}

void vkInit::make_frame_command_buffers(commandBufferInputChunk inputChunk, bool debugMode) {
	
	VkCommandBufferAllocateInfo mainAllocInfo = {};
	mainAllocInfo.commandPool = inputChunk.commandPool;
	mainAllocInfo.level = VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	mainAllocInfo.commandBufferCount = 1;
	mainAllocInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;


	for (int i = 0; i < inputChunk.frames.size(); ++i) {
		VkResult result = vkAllocateCommandBuffers(inputChunk.device, &mainAllocInfo, &inputChunk.frames[i].mainCommandBuffer);
		if (result != VK_SUCCESS) {
			std::cout << "FAILED!!! Allocated command buffer for frame " << i << ", error code: " << result << std::endl;
		}
		else {
			if (debugMode)
				std::cout << "Allocated command buffers for frame " << i << std::endl;
		}
	}
	
}

void vkInit::make_frame_compute_command_buffers(commandBufferInputChunk inputChunk, bool debugMode){
	VkCommandBufferAllocateInfo mainAllocInfo = {};
	mainAllocInfo.commandPool = inputChunk.commandPool;
	mainAllocInfo.level = VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	mainAllocInfo.commandBufferCount = 1;
	mainAllocInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;


	for (int i = 0; i < inputChunk.frames.size(); ++i) {
		VkResult result = vkAllocateCommandBuffers(inputChunk.device, &mainAllocInfo, &inputChunk.frames[i].computeCommandBuffer);
		if (result != VK_SUCCESS) {
			std::cout << "FAILED!!! Allocated command buffer for frame " << i << ", error code: " << result << std::endl;
		}
		else {
			if (debugMode)
				std::cout << "Allocated command buffers for frame " << i << std::endl;
		}
	}
}

void vkInit::make_postprocess_frame_command_buffers(commandBufferInputChunk inputChunk, bool debugMode) {
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.commandPool = inputChunk.commandPool;
	allocInfo.level = VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;
}

void vkInit::flushCommandBuffer(VkDevice device, VkCommandBuffer commandBuffer, VkQueue queue, VkCommandPool pool, bool free) {
	if (commandBuffer == VK_NULL_HANDLE)
	{
		return;
	}

	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	submitInfo.pNext = nullptr;
	// Create fence to ensure that the command buffer has finished executing
	VkFenceCreateInfo fenceInfo;
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.pNext = nullptr;
	fenceInfo.flags = VkFenceCreateFlags() | VkFenceCreateFlagBits::VK_FENCE_CREATE_SIGNALED_BIT;
	VkFence fence;
	vkCreateFence(device, &fenceInfo, nullptr, &fence);
	// Submit to the queue
	vkQueueSubmit(queue, 1, &submitInfo, fence);
	// Wait for the fence to signal that command buffer has finished executing
	vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX);
	vkDestroyFence(device, fence, nullptr);
	if (free)
	{
		vkFreeCommandBuffers(device, pool, 1, &commandBuffer);
	}
}
