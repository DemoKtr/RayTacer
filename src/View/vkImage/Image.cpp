#include "View/vkImage/Image.h"
#include <View/vkUtil/memory.h>
#include <View/vkUtil/singleTimeCommands.h>

void vkImage::make_image(ImageInputChunk input) {
	/*
	typedef struct VkImageCreateInfo {
		VkStructureType          sType;
		const void* pNext;
		VkImageCreateFlags       flags;
		VkImageType              imageType;
		VkFormat                 format;
		VkExtent3D               extent;
		uint32_t                 mipLevels;
		uint32_t                 arrayLayers;
		VkSampleCountFlagBits    samples;
		VkImageTiling            tiling;
		VkImageUsageFlags        usage;
		VkSharingMode            sharingMode;
		uint32_t                 queueFamilyIndexCount;
		const uint32_t* pQueueFamilyIndices;
		VkImageLayout            initialLayout;
	} VkImageCreateInfo;
	*/

	VkImageCreateInfo imageInfo;
	imageInfo.flags = VkImageCreateFlagBits() | input.flags;
	if (input.height > 1)
		imageInfo.imageType = VkImageType::VK_IMAGE_TYPE_2D;
	else imageInfo.imageType = VkImageType::VK_IMAGE_TYPE_1D;
	imageInfo.extent = VkExtent3D(input.width, input.height, 1);
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = input.arrayCount;
	imageInfo.format = input.format;
	imageInfo.tiling = input.tiling;
	imageInfo.initialLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = input.usage;
	imageInfo.sharingMode = VkSharingMode::VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;
	
	vkCreateImage(input.logicalDevice, &imageInfo, nullptr, &input.image);
	
}

VkDeviceMemory vkImage::make_image_memory(ImageInputChunk input, VkImage image) {
	VkMemoryRequirements requirements; vkGetImageMemoryRequirements(input.logicalDevice, input.image, &requirements);

	VkMemoryAllocateInfo allocation;
	allocation.allocationSize = requirements.size;
	allocation.memoryTypeIndex = vkUtil::findMemoryTypeIndex(
		input.physicalDevice, requirements.memoryTypeBits, VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);

	
		VkDeviceMemory imageMemory;
		vkAllocateMemory(input.logicalDevice, &allocation, nullptr, &imageMemory);
		vkBindImageMemory(input.logicalDevice, image, imageMemory, 0);
		return imageMemory;
	
}

void vkImage::transition_image_layout(ImageLayoutTransitionJob job) {
	vkUtil::startJob(job.commandBuffer);

	/*
	typedef struct VkImageSubresourceRange {
		VkImageAspectFlags    aspectMask;
		uint32_t              baseMipLevel;
		uint32_t              levelCount;
		uint32_t              baseArrayLayer;
		uint32_t              layerCount;
	} VkImageSubresourceRange;
	*/
	VkImageSubresourceRange access;
	access.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
	access.baseMipLevel = 0;
	access.levelCount = 1;
	access.baseArrayLayer = 0;
	access.layerCount = job.arrayCount;

	/*
	typedef struct VkImageMemoryBarrier {
		VkStructureType            sType;
		const void* pNext;
		VkAccessFlags              srcAccessMask;
		VkAccessFlags              dstAccessMask;
		VkImageLayout              oldLayout;
		VkImageLayout              newLayout;
		uint32_t                   srcQueueFamilyIndex;
		uint32_t                   dstQueueFamilyIndex;
		VkImage                    image;
		VkImageSubresourceRange    subresourceRange;
	} VkImageMemoryBarrier;
	*/
	VkImageMemoryBarrier barrier;
	barrier.oldLayout = job.oldLayout;
	barrier.newLayout = job.newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = job.image;
	barrier.subresourceRange = access;

	VkPipelineStageFlags sourceStage, destinationStage;

	if (job.oldLayout == VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED
		&& job.newLayout == VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {

		barrier.srcAccessMask = VkAccessFlagBits::VK_ACCESS_NONE_KHR;
		barrier.dstAccessMask = VkAccessFlagBits::VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else {

		barrier.srcAccessMask = VkAccessFlagBits::VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VkAccessFlagBits::VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}

	vkCmdPipelineBarrier(
		job.commandBuffer,
		sourceStage,
		destinationStage,
		0, // VkDependencyFlags (domyúlnie 0)
		0, nullptr, // Memory Barriers
		0, nullptr, // Buffer Barriers
		1, &barrier // Image Barrier
	);

	vkUtil::endJob(job.commandBuffer, job.queue);
}

void vkImage::copy_buffer_to_image(BufferImageCopyJob job) {
	vkUtil::startJob(job.commandBuffer);

	/*
	typedef struct VkBufferImageCopy {
		VkDeviceSize                bufferOffset;
		uint32_t                    bufferRowLength;
		uint32_t                    bufferImageHeight;
		VkImageSubresourceLayers    imageSubresource;
		VkOffset3D                  imageOffset;
		VkExtent3D                  imageExtent;
	} VkBufferImageCopy;
	*/
	VkBufferImageCopy copy;
	copy.bufferOffset = 0;
	copy.bufferRowLength = 0;
	copy.bufferImageHeight = 0;

	VkImageSubresourceLayers access;
	access.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
	access.mipLevel = 0;
	access.baseArrayLayer = 0;
	access.layerCount = job.arrayCount;
	copy.imageSubresource = access;

	copy.imageOffset = VkOffset3D(0, 0, 0);
	copy.imageExtent = VkExtent3D(
		job.width,
		job.height,
		job.depth
	);

	vkCmdCopyBufferToImage(
		job.commandBuffer,
		job.srcBuffer,
		job.dstImage,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1, // Liczba kopii
		&copy // Wskaünik do struktury VkBufferImageCopy
	);

	vkUtil::endJob(job.commandBuffer, job.queue);
}

void vkImage::make_image_view(VkDevice logicalDevice, VkImage image,VkImageView& imageView ,VkFormat format, VkImageAspectFlags aspect, VkImageViewType type, uint32_t arrayCount) {

	/*
	* ImageViewCreateInfo( VULKAN_HPP_NAMESPACE::ImageViewCreateFlags flags_ = {},
		VULKAN_HPP_NAMESPACE::Image                image_ = {},
		VULKAN_HPP_NAMESPACE::ImageViewType    viewType_  = VULKAN_HPP_NAMESPACE::ImageViewType::e1D,
		VULKAN_HPP_NAMESPACE::Format           format_    = VULKAN_HPP_NAMESPACE::Format::eUndefined,
		VULKAN_HPP_NAMESPACE::ComponentMapping components_            = {},
		VULKAN_HPP_NAMESPACE::ImageSubresourceRange subresourceRange_ = {} ) VULKAN_HPP_NOEXCEPT
	*/

	VkImageViewCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.image = image;
	createInfo.viewType = type;
	createInfo.format = format;
	createInfo.components.r = VkComponentSwizzle::VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.g = VkComponentSwizzle::VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.b = VkComponentSwizzle::VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.a = VkComponentSwizzle::VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.subresourceRange.aspectMask = aspect;
	createInfo.subresourceRange.baseMipLevel = 0;
	createInfo.subresourceRange.levelCount = 1;
	createInfo.subresourceRange.baseArrayLayer = 0;
	createInfo.subresourceRange.layerCount = arrayCount;

	vkCreateImageView(logicalDevice, &createInfo, nullptr, &imageView);
}

VkFormat vkImage::find_supported_format(VkPhysicalDevice physicalDevice, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {

	for (VkFormat format : candidates) {
		VkFormatProperties properties;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &properties);
		/*
		typedef struct VkFormatProperties {
			VkFormatFeatureFlags    linearTilingFeatures;
			VkFormatFeatureFlags    optimalTilingFeatures;
			VkFormatFeatureFlags    bufferFeatures;
		} VkFormatProperties;
		*/

		if (tiling == VkImageTiling::VK_IMAGE_TILING_LINEAR
			&& (properties.linearTilingFeatures & features) == features) {
			return format;
		}

		if (tiling == VkImageTiling::VK_IMAGE_TILING_OPTIMAL
			&& (properties.optimalTilingFeatures & features) == features) {
			return format;
		}
		std::cout << "Error can't find suitable format" << std::endl;

	}
}
