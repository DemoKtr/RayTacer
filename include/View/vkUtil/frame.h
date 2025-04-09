#pragma once
#include "config.h"
#include "descriptorsBuffers.h"
#include "glm/glm.hpp"

namespace vkAccelerationStructure {
	class VertexMenagerie;
}

namespace vkUtil {

	struct UBO {
		glm::mat4 inverseView;
		glm::mat4 inverseProj;
	};

	struct Light {
		glm::vec4 position;
		glm::vec4 intensity;
	};
	
	struct Material {
		glm::vec3 color;
		float shininess;
		float ambientCoefficient;
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

		Buffer light;
		void* lightDataWriteLocation;
		VkDescriptorBufferInfo lightDescritorBufferInfo;
		Light lightData;
		
		Buffer material;
		void* materialDataWriteLocation;
		VkDescriptorBufferInfo materialDescritorBufferInfo;
		Material materialData;
		
		Buffer normalsAndTextcords;
		void* normalsAndTextcordsDataWriteLocation;
		VkDescriptorBufferInfo normalsAndTextcordsDescritorBufferInfo;
		std::vector<float> normalsAndTextcordsData;
		
		Buffer offsets;
		void* offsetsDataWriteLocation;
		VkDescriptorBufferInfo offsetsBufferInfo;
		std::vector<float> offsetsData;

		void destroy();
		void make_descriptors_resources(vkAccelerationStructure::VertexMenagerie *vertexMenagerie);
		void write_descriptors(VkAccelerationStructureKHR handle, uint32_t size);
	};

}
