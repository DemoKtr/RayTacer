#pragma once
#include "config.h"
#include <View/vkUtil/memory.h>


namespace vkAccelerationStructure {
	struct AccelerationStructure {
		VkAccelerationStructureKHR handle;
		uint64_t deviceAddress = 0;
		VkDeviceMemory memory;
		VkBuffer buffer;
	};

	struct RayTracingScratchBuffer
	{
		uint64_t deviceAddress = 0;
		VkBuffer handle = VK_NULL_HANDLE;
		VkDeviceMemory memory = VK_NULL_HANDLE;
	};

	RayTracingScratchBuffer createScratchBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkDeviceSize size);


	void deleteScratchBuffer(VkDevice device, RayTracingScratchBuffer& scratchBuffer);

	void createAccelerationStructureBuffer(VkPhysicalDevice physicalDevice, VkDevice device, AccelerationStructure& accelerationStructure, VkAccelerationStructureBuildSizesInfoKHR buildSizeInfo);

}


