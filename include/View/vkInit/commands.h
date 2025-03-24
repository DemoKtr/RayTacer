#pragma once
#include "config.h"
#include <View/vkUtil/frame.h>
#include <vector>

namespace vkInit {
	struct commandBufferInputChunk {
		VkDevice device;
		VkCommandPool& commandPool;
		std::vector<vkUtil::SwapChainFrame>& frames;
		
	};



	void make_command_pool(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool& commandPool,VkSurfaceKHR surface, bool debugMode);
	void make_compute_command_pool(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool& commandPool,VkSurfaceKHR surface, bool debugMode);
	void make_transfer_command_pool(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool& commandPool, VkSurfaceKHR surface, bool debugMode);

	void make_command_buffer(commandBufferInputChunk inputChunk, VkCommandBuffer& commandBuffer, bool debugMode);



	void make_frame_command_buffers(commandBufferInputChunk inputChunk, bool debugMode);
	void make_frame_compute_command_buffers(commandBufferInputChunk inputChunk, bool debugMode);
	void make_postprocess_frame_command_buffers(commandBufferInputChunk inputChunk, bool debugMode);


	void flushCommandBuffer(VkDevice device ,VkCommandBuffer commandBuffer, VkQueue queue, VkCommandPool pool, bool free);

}