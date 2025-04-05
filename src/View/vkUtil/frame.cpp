#include "View/vkUtil/frame.h"
#include <View/vkUtil/memory.h>

void vkUtil::SwapChainFrame::destroy() {
	vkDestroyImageView(logicalDevice, mainImageView, nullptr);

	vkDestroySemaphore(logicalDevice, renderFinished, nullptr);
	vkDestroySemaphore(logicalDevice, imageAvailable, nullptr);
	vkDestroySemaphore(logicalDevice, computeFinished, nullptr);
	vkDestroyFence(logicalDevice, inFlight, nullptr);
}

void vkUtil::SwapChainFrame::make_descriptors_resources() {
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.flags = VkSamplerCreateFlags();
	samplerInfo.minFilter = VkFilter::VK_FILTER_LINEAR;
	samplerInfo.magFilter = VkFilter::VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeV = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeW = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

	samplerInfo.anisotropyEnable = VK_FALSE;
	samplerInfo.maxAnisotropy = 1.0f;

	samplerInfo.borderColor = VkBorderColor::VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VkCompareOp::VK_COMPARE_OP_ALWAYS;

	samplerInfo.mipmapMode = VkSamplerMipmapMode::VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;

	vkCreateSampler(logicalDevice, &samplerInfo, nullptr, &sampler);

	BufferInputChunk input;
	input.logicalDevice = logicalDevice;
	input.memoryProperties = VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	input.physicalDevice = physicalDevice;
	input.size = sizeof(UBO);
	input.usage = VkBufferUsageFlagBits::VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	vkUtil::createBuffer(input,ubo);
	vkMapMemory(logicalDevice, ubo.bufferMemory, 0, sizeof(UBO), 0, &uboDataWriteLocation);

	uboDescritorBufferInfo.buffer = ubo.buffer;
	uboDescritorBufferInfo.offset = 0;
	uboDescritorBufferInfo.range = sizeof(UBO);


}

void vkUtil::SwapChainFrame::write_descriptors(VkAccelerationStructureKHR handle, uint32_t size) {


	VkDescriptorImageInfo imageDescriptor2;
	imageDescriptor2.imageLayout = VkImageLayout::VK_IMAGE_LAYOUT_GENERAL;
	imageDescriptor2.imageView = postProcessImageView;
	imageDescriptor2.sampler = nullptr;

	VkWriteDescriptorSet writeInfo2;
	writeInfo2.sType = VkStructureType::VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeInfo2.dstSet = RayGenDescriptorSet;
	writeInfo2.dstBinding = 1;
	writeInfo2.dstArrayElement = 0;
	writeInfo2.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	writeInfo2.descriptorCount = 1;
	writeInfo2.pImageInfo = &imageDescriptor2;
	writeInfo2.pNext = nullptr;
	vkUpdateDescriptorSets(logicalDevice, 1, &writeInfo2, 0, nullptr);


	
	VkWriteDescriptorSetAccelerationStructureKHR accelerationStructureInfo{};
	accelerationStructureInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
	accelerationStructureInfo.accelerationStructureCount = 1;
	accelerationStructureInfo.pAccelerationStructures = &handle; // Tw�j bufor AS

	
	VkWriteDescriptorSet writeInfo3;
	writeInfo3.sType = VkStructureType::VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeInfo3.dstSet = RayGenDescriptorSet;
	writeInfo3.dstBinding = 0;
	writeInfo3.dstArrayElement = 0;
	writeInfo3.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
	writeInfo3.descriptorCount = 1;
	writeInfo3.pNext = &accelerationStructureInfo;

	vkUpdateDescriptorSets(logicalDevice, 1, &writeInfo3, 0, nullptr);

	VkWriteDescriptorSet writeInfo4;
	writeInfo4.sType = VkStructureType::VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeInfo4.dstSet = RayGenDescriptorSet;
	writeInfo4.dstBinding = 2;
	writeInfo4.dstArrayElement = 0;
	writeInfo4.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writeInfo4.descriptorCount = 1;
	writeInfo4.pBufferInfo = &uboDescritorBufferInfo;
	writeInfo4.pNext = nullptr;

	vkUpdateDescriptorSets(logicalDevice, 1, &writeInfo4, 0, nullptr);

	VkDescriptorImageInfo imageDescriptor;

	imageDescriptor.imageLayout = VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageDescriptor.imageView = postProcessImageView;
	imageDescriptor.sampler = sampler;

	VkWriteDescriptorSet writeInfo;
	writeInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeInfo.dstSet = postProcessDescriptorSet;
	writeInfo.dstBinding = 0;
	writeInfo.dstArrayElement = 0;
	writeInfo.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	writeInfo.descriptorCount = 1;
	writeInfo.pImageInfo = &imageDescriptor;
	writeInfo.pNext = nullptr;
	
	vkUpdateDescriptorSets(logicalDevice, 1, &writeInfo, 0,  nullptr);




}
