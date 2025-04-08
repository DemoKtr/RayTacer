#pragma once
#include "config.h"
#include "View/vkImage/Image.h"


namespace vkImage {

	class Texture {
		int width, height, channels;
		VkDevice logicalDevice;
		VkPhysicalDevice physicalDevice;

		std::vector<stbi_uc*> pixels;
		std::vector<float> data;
		std::vector<std::string> texturesNames;

		VkImage image;
		VkDeviceMemory imageMemory;
		VkImageView imageView;
		VkSampler sampler;

		//Resource Descriptors
		VkDescriptorSetLayout layout;
		VkDescriptorSet descriptorSet;
		VkDescriptorPool descriptorPool;

		VkCommandBuffer commandBuffer;
		VkQueue queue;

		void load();
		void populate();

		void make_view();

		void make_view(VkFormat format, VkImageViewType type);
		void make_sampler();
		void make_descriptor_set();

	public:

		const char* filename;

		VkDescriptorSet getDescriptorSet();
		VkSampler getSampler();

		Texture(TextureInputChunk info);
		~Texture();

		void wrie_to_descriptor_set(VkDescriptorSet& desc);
		void useTexture(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout);
		void useTexture(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, int layoutIndex);


	};

}