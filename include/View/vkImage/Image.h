#pragma once
#include "config.h"
#include <vector>

namespace vkImage {	

	struct ImageInputChunk {
		VkDevice logicalDevice;
		VkPhysicalDevice physicalDevice;
		int width, height;
		VkImageTiling tiling;
		VkImageUsageFlags usage;
		VkMemoryPropertyFlags memoryProperties;
		VkFormat format;
		uint32_t arrayCount;
		VkImageCreateFlags flags;
		VkImage& image;
	};

	struct Image3DInputChunk {
		VkDevice logicalDevice;
		VkPhysicalDevice physicalDevice;
		int width, height, depth;
		VkImageTiling tiling;
		VkImageUsageFlags usage;
		VkMemoryPropertyFlags memoryProperties;
		VkFormat format;
		uint32_t arrayCount;
		VkImageCreateFlags flags;
	};



	struct BufferImageCopyJob {
		VkCommandBuffer commandBuffer;
		VkQueue queue;
		VkBuffer srcBuffer;
		VkImage dstImage;
		int width, height;
		int depth = 1.0f;
		uint32_t arrayCount;

	};

	struct ImageLayoutTransitionJob {
		VkCommandBuffer commandBuffer;
		VkQueue queue;
		VkImage image;
		VkImageLayout oldLayout, newLayout;
		uint32_t arrayCount;
	};

	void make_image(ImageInputChunk input);
	VkImage make_image(Image3DInputChunk input);

	VkDeviceMemory make_image_memory(ImageInputChunk input, VkImage image);
	VkDeviceMemory make_image_memory(Image3DInputChunk input, VkImage image);
	void transition_image_layout(ImageLayoutTransitionJob job);
	void copy_buffer_to_image(BufferImageCopyJob job);
	void make_image_view(VkDevice logicalDevice, VkImage image, VkImageView& imageView, VkFormat format, VkImageAspectFlags aspect, VkImageViewType type, uint32_t arrayCount);

	VkFormat find_supported_format(
		VkPhysicalDevice physicalDevice, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features
	);






}