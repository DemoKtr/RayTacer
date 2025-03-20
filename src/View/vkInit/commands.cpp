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

void vkInit::make_command_buffer(commandBufferInputChunk inputChunk, bool debugMode) {
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.commandPool = inputChunk.commandPool;
	allocInfo.level = VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;
	allocInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	VkResult result = vkAllocateCommandBuffers(inputChunk.device, &allocInfo, &inputChunk.commandBuffer);
	if (result != VK_SUCCESS) {
		std::cout << "FAILED!!! Allocated main command buffer for frame, error code: " << result << std::endl;
		
	}
	if (debugMode)
		std::cout << "Allocated main command buffer for frame" << std::endl;
	
}

void vkInit::make_imgui_frame_command_buffers(commandBufferInputChunk inputChunk, bool debugMode) {
	/*
	VkCommandBufferAllocateInfo mainAllocInfo = {};
	mainAllocInfo.commandPool = inputChunk.commandPool;
	mainAllocInfo.level = VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	mainAllocInfo.commandBufferCount = 1;

	VkCommandBufferAllocateInfo seccAllocInfo = {};
	seccAllocInfo.commandPool = inputChunk.commandPool;
	seccAllocInfo.level = VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_SECONDARY;
	seccAllocInfo.commandBufferCount = 1;



	for (int i = 0; i < inputChunk.frames.size(); ++i) {
		VkResult result = vkAllocateCommandBuffers(inputChunk.device, &allocInfo, commandBuffers);
		if (result != VK_SUCCESS) {
			std::cout << "FAILED!!! Allocated command buffer for frame " << frameIndex << ", error code: " << result << std::endl;
		}
		else {
			if (debugMode)
				std::cout << "Allocated command buffers for frame " << frameIndex << std::endl;
		}
	}
	*/
}

void vkInit::make_postprocess_frame_command_buffers(commandBufferInputChunk inputChunk, bool debugMode) {
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.commandPool = inputChunk.commandPool;
	allocInfo.level = VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;
}
