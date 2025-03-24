#pragma once
#include "config.h"
#include "descriptorsBuffers.h"
#include "glm/glm.hpp"
namespace vkUtil {

	struct UBO {
		glm::mat4 inverseView;
		glm::mat4 inverseProj;
	};

	class SwapChainFrame {
	public:
		VkDevice logicalDevice;
		VkPhysicalDevice physicalDevice;
		VkSemaphore imageAvailable, renderFinished, computeFinished;
		VkFence inFlight;
		VkImage mainImage;
		VkImageView mainImageView;
		

		//Postprocess
		VkImage postProcessImage;
		VkImageView postProcessImageView;
		VkDeviceMemory postProcessImageBufferMemory;


		VkSampler sampler;
		VkDescriptorSet postProcessDescriptorSet;
		VkDescriptorSet RayCastDescriptorSet;
		VkDescriptorSet RayGenDescriptorSet;


		VkCommandBuffer mainCommandBuffer;
		VkCommandBuffer computeCommandBuffer;

		///
		Buffer ubo;
		void* uboDataWriteLocation;
		VkDescriptorBufferInfo uboDescritorBufferInfo;
		UBO uboData;


		void destroy();
		void make_descriptors_resources();
		void write_descriptors(VkAccelerationStructureKHR handle, uint32_t size);
	};

}
