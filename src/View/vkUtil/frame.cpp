#include "View/vkUtil/frame.h"
#include <View/vkUtil/memory.h>

#include "View/vkAccelerationStructures/AccelerationStructureMenagerie.h"

void vkUtil::SwapChainFrame::destroy() {
	vkDestroyImageView(logicalDevice, mainImageView, nullptr);

	vkDestroySemaphore(logicalDevice, renderFinished, nullptr);
	vkDestroySemaphore(logicalDevice, imageAvailable, nullptr);
	vkDestroySemaphore(logicalDevice, computeFinished, nullptr);
	vkDestroyFence(logicalDevice, inFlight, nullptr);
}

void vkUtil::SwapChainFrame::make_descriptors_resources(vkAccelerationStructure::VertexMenagerie *vertexMenagerie) {
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

	BufferInputChunk inputCamera;
	inputCamera.logicalDevice = logicalDevice;
	inputCamera.memoryProperties = VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	inputCamera.physicalDevice = physicalDevice;
	inputCamera.size = sizeof(UBO);
	inputCamera.usage = VkBufferUsageFlagBits::VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	vkUtil::createBuffer(inputCamera,ubo);
	vkMapMemory(logicalDevice, ubo.bufferMemory, 0, sizeof(UBO), 0, &uboDataWriteLocation);

	uboDescritorBufferInfo.buffer = ubo.buffer;
	uboDescritorBufferInfo.offset = 0;
	uboDescritorBufferInfo.range = sizeof(UBO);


	BufferInputChunk inputLight;
	inputLight.logicalDevice = logicalDevice;
	inputLight.memoryProperties = VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	inputLight.physicalDevice = physicalDevice;
	inputLight.size = sizeof(Light);
	inputLight.usage = VkBufferUsageFlagBits::VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	vkUtil::createBuffer(inputLight,light);
	vkMapMemory(logicalDevice, light.bufferMemory, 0, sizeof(Light), 0, &lightDataWriteLocation);

	lightDescritorBufferInfo.buffer = light.buffer;
	lightDescritorBufferInfo.offset = 0;
	lightDescritorBufferInfo.range = sizeof(Light);


	BufferInputChunk inputMaterial;
	inputMaterial.logicalDevice = logicalDevice;
	inputMaterial.memoryProperties = VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	inputMaterial.physicalDevice = physicalDevice;
	inputMaterial.size = sizeof(Material);
	inputMaterial.usage = VkBufferUsageFlagBits::VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	vkUtil::createBuffer(inputMaterial,material);
	vkMapMemory(logicalDevice, material.bufferMemory, 0, sizeof(Material), 0, &materialDataWriteLocation);

	materialDescritorBufferInfo.buffer = material.buffer;
	materialDescritorBufferInfo.offset = 0;
	materialDescritorBufferInfo.range = sizeof(Material);
	


	BufferInputChunk inpuNormalsAndTextcords;
	inpuNormalsAndTextcords.logicalDevice = logicalDevice;
	inpuNormalsAndTextcords.memoryProperties = VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	inpuNormalsAndTextcords.physicalDevice = physicalDevice;
	inpuNormalsAndTextcords.size = vertexMenagerie->totalExtraBLASBufferSize;
	inpuNormalsAndTextcords.usage = VkBufferUsageFlagBits::VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	vkUtil::createBuffer(inpuNormalsAndTextcords,normalsAndTextcords);
	vkMapMemory(logicalDevice, normalsAndTextcords.bufferMemory, 0, vertexMenagerie->totalExtraBLASBufferSize, 0, &normalsAndTextcordsDataWriteLocation);
	
	normalsAndTextcordsDescritorBufferInfo.buffer = normalsAndTextcords.buffer;
	normalsAndTextcordsDescritorBufferInfo.offset = 0;
	normalsAndTextcordsDescritorBufferInfo.range = vertexMenagerie->totalExtraBLASBufferSize;
	

	BufferInputChunk inputOffsets;
	inputOffsets.logicalDevice = logicalDevice;
	inputOffsets.memoryProperties = VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	inputOffsets.physicalDevice = physicalDevice;
	inputOffsets.size = sizeof(float) * vertexMenagerie->extraBLASoffsets.size();
	inputOffsets.usage = VkBufferUsageFlagBits::VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	vkUtil::createBuffer(inputOffsets,offsets);
	vkMapMemory(logicalDevice, offsets.bufferMemory, 0,  sizeof(float) * vertexMenagerie->extraBLASoffsets.size(), 0, &offsetsDataWriteLocation);
	
	offsetsBufferInfo.buffer = offsets.buffer;
	offsetsBufferInfo.offset = 0;
	offsetsBufferInfo.range = sizeof(float) * vertexMenagerie->extraBLASoffsets.size();
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

	VkWriteDescriptorSet writeInfo5;
	writeInfo5.sType = VkStructureType::VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeInfo5.dstSet = RayGenDescriptorSet;
	writeInfo5.dstBinding = 3;
	writeInfo5.dstArrayElement = 0;
	writeInfo5.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writeInfo5.descriptorCount = 1;
	writeInfo5.pBufferInfo = &lightDescritorBufferInfo;
	writeInfo5.pNext = nullptr;
	
	VkWriteDescriptorSet writeInfo6;
	writeInfo6.sType = VkStructureType::VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeInfo6.dstSet = RayGenDescriptorSet;
	writeInfo6.dstBinding = 4;
	writeInfo6.dstArrayElement = 0;
	writeInfo6.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writeInfo6.descriptorCount = 1;
	writeInfo6.pBufferInfo = &materialDescritorBufferInfo;
	writeInfo6.pNext = nullptr;
	
	
	VkWriteDescriptorSet writeInfo7;
	writeInfo7.sType = VkStructureType::VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeInfo7.dstSet = RayGenDescriptorSet;
	writeInfo7.dstBinding = 5;
	writeInfo7.dstArrayElement = 0;
	writeInfo7.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	writeInfo7.descriptorCount = 1;
	writeInfo7.pBufferInfo = &normalsAndTextcordsDescritorBufferInfo;
	writeInfo7.pNext = nullptr;

	VkWriteDescriptorSet writeInfo8;
	writeInfo8.sType = VkStructureType::VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeInfo8.dstSet = RayGenDescriptorSet;
	writeInfo8.dstBinding = 6;
	writeInfo8.dstArrayElement = 0;
	writeInfo8.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	writeInfo8.descriptorCount = 1;
	writeInfo8.pBufferInfo = &offsetsBufferInfo;
	writeInfo8.pNext = nullptr;
	
	VkWriteDescriptorSet writeInfos[] = {writeInfo4, writeInfo5, writeInfo6,writeInfo7,writeInfo8};

	
	vkUpdateDescriptorSets(logicalDevice, 5, writeInfos, 0, nullptr);
	
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
