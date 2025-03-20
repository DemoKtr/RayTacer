#pragma once
#include "config.h"
#include <View/vkUtil/frame.h>
#include <vector>

namespace vkInit {
	struct commandBufferInputChunk {
		VkDevice device;
		VkCommandPool commandPool;
		std::vector<vkUtil::SwapChainFrame>& frames;
		VkCommandBuffer& commandBuffer;
	};



	void make_command_pool(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool& commandPool,VkSurfaceKHR surface, bool debugMode);
	void make_compute_command_pool(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool& commandPool,VkSurfaceKHR surface, bool debugMode);
	void make_transfer_command_pool(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool& commandPool, VkSurfaceKHR surface, bool debugMode);

	void make_command_buffer(commandBufferInputChunk inputChunk, bool debugMode);



	void make_imgui_frame_command_buffers(commandBufferInputChunk inputChunk, bool debugMode);
	void make_postprocess_frame_command_buffers(commandBufferInputChunk inputChunk, bool debugMode);

}