#include "View/vkImage/Texture.h"
#include <View/vkUtil/memory.h>
#include <View/vkInit/descriptors.h>

#define STB_IMAGE_IMPLEMENTATION
#include "View/vkImage/stb_image.h"
vkImage::Texture::Texture(TextureInputChunk info) {

	logicalDevice = info.logicalDevice;
	physicalDevice = info.physicalDevice;
	texturesNames = info.texturesNames;
	filename = info.filenames;
	commandBuffer = info.commandBuffer;
	queue = info.queue;
	layout = info.layout;
	descriptorPool = info.descriptorPool;


	load();
	channels = 4;
	ImageInputChunk imageInput;
	imageInput.logicalDevice = logicalDevice;
	imageInput.physicalDevice = physicalDevice;
	imageInput.width = width;
	imageInput.height = height;
	imageInput.format = VkFormat::VK_FORMAT_R8G8B8A8_UNORM;
	imageInput.arrayCount = pixels.size();
	imageInput.tiling = VkImageTiling::VK_IMAGE_TILING_OPTIMAL;
	imageInput.usage = VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_DST_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_SAMPLED_BIT;
	imageInput.memoryProperties = VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	imageInput.image = &image;
	imageInput.flags = 0;
	make_image(imageInput);
	imageMemory = make_image_memory(imageInput, image);

	populate();

	for (stbi_uc* pix : pixels) {
		free(pix);
	}


	make_view();

	make_sampler();

	make_descriptor_set();
}

vkImage::Texture::~Texture() {

}

void vkImage::Texture::useTexture(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout) {
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipelineLayout, 1, 1, &descriptorSet, 0, 0);
}

void vkImage::Texture::load() {
	size_t i = 0;
	if (filename == nullptr) {
		for (std::string f : texturesNames) {
			pixels.push_back(stbi_load(texturesNames[i].c_str(), &width, &height, &channels, STBI_rgb_alpha));
			if (!pixels[i++]) {
				std::cout << "Unable to load: " << f << std::endl;
			}
		}
	}
	else {
		pixels.push_back(stbi_load(filename, &width, &height, &channels, STBI_rgb_alpha));
		if (!pixels[i++]) {
			std::cout << "Unable to load: " << filename << std::endl;
		}
	}
}

void vkImage::Texture::populate() {

	BufferInputChunk input;
	input.logicalDevice = logicalDevice;
	input.physicalDevice = physicalDevice;
	input.memoryProperties = VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
	input.usage = VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	size_t totalSize = 0;
	for (const auto& image : pixels) {
		totalSize += width * height * channels; // assuming 4 bytes per pixel (e.g., RGBA format)
	}
	std::vector<stbi_uc> mergedPixels;
	mergedPixels.reserve(totalSize);
	input.size = totalSize;

	// 3. Skopiuj wszystkie dane z pixels do mergedPixels
	for (const auto& image : pixels) {
		mergedPixels.insert(mergedPixels.end(), image, image + width * height * channels);
	}

	Buffer stagingBuffer;
	vkUtil::createBuffer(input, stagingBuffer);

	//...then fill it,
	void* writeLocation = nullptr;
	
	vkMapMemory(logicalDevice, stagingBuffer.bufferMemory, 0, input.size, 0, &writeLocation);
	if (writeLocation) {
		memcpy(writeLocation, mergedPixels.data(), totalSize);
		vkUnmapMemory(logicalDevice, stagingBuffer.bufferMemory);
	}
	else {
		std::cerr << "Memory mapping failed!" << std::endl;
	}

	//then transfer it to image memory
	ImageLayoutTransitionJob transitionJob;
	transitionJob.commandBuffer = commandBuffer;
	transitionJob.queue = queue;
	transitionJob.image = image;
	transitionJob.oldLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
	transitionJob.newLayout = VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	transitionJob.arrayCount = pixels.size();

	transition_image_layout(transitionJob);

	BufferImageCopyJob copyJob;
	copyJob.commandBuffer = commandBuffer;
	copyJob.queue = queue;
	copyJob.srcBuffer = stagingBuffer.buffer;
	copyJob.dstImage = image;
	copyJob.width = width;
	copyJob.height = height;
	copyJob.arrayCount = pixels.size();
	copy_buffer_to_image(copyJob);

	transitionJob.oldLayout = VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	transitionJob.newLayout = VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	transition_image_layout(transitionJob);

	//Now the staging buffer can be destroyed
	vkFreeMemory(logicalDevice, stagingBuffer.bufferMemory, nullptr);
	vkDestroyBuffer(logicalDevice, stagingBuffer.buffer, nullptr);

}

void vkImage::Texture::make_view() {
	if (filename == nullptr) {
		make_image_view(logicalDevice, image,imageView ,VkFormat::VK_FORMAT_R8G8B8A8_UNORM, VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT, VkImageViewType::VK_IMAGE_VIEW_TYPE_2D_ARRAY, pixels.size());
	}
	else {
		make_image_view(logicalDevice, image, imageView, VkFormat::VK_FORMAT_R8G8B8A8_UNORM, VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT, VkImageViewType::VK_IMAGE_VIEW_TYPE_2D, pixels.size());
	}
}

void vkImage::Texture::make_sampler() {
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.flags = VkSamplerCreateFlags();
	samplerInfo.minFilter = VkFilter::VK_FILTER_NEAREST;
	samplerInfo.magFilter = VkFilter::VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_REPEAT;

	samplerInfo.anisotropyEnable = false;
	samplerInfo.maxAnisotropy = 1.0f;

	samplerInfo.borderColor = VkBorderColor::VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = false;
	samplerInfo.compareEnable = false;
	samplerInfo.compareOp = VkCompareOp::VK_COMPARE_OP_ALWAYS;

	samplerInfo.mipmapMode = VkSamplerMipmapMode::VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;
	samplerInfo.pNext = nullptr;
	vkCreateSampler(logicalDevice, &samplerInfo, nullptr, &sampler);

}

void vkImage::Texture::make_descriptor_set() {
	 vkInit::allocate_descriptor_set(logicalDevice, descriptorSet,descriptorPool, layout);

	VkDescriptorImageInfo imageDescriptor;
	
	imageDescriptor.imageLayout = VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageDescriptor.imageView = imageView;
	imageDescriptor.sampler = sampler;

	VkWriteDescriptorSet descriptorWrite{};
	descriptorWrite.sType = VkStructureType::VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = descriptorSet;
	descriptorWrite.dstBinding = 0;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pImageInfo = &imageDescriptor;
	descriptorWrite.pNext = nullptr;
	vkUpdateDescriptorSets(logicalDevice, 1, &descriptorWrite, 0, nullptr);
}
