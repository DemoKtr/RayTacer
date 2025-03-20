#pragma once
#include "config.h"
#include <vector>
namespace vkInit {




	void choose_physical_device(VkInstance& instance,VkPhysicalDevice& physicalDevice ,bool debugMode);
	bool isSuitable(const VkPhysicalDevice& physicalDevice, bool debugMode);
	bool checkDeviceExtensionSupport(
		const VkPhysicalDevice& physicalDevice,
		const std::vector<const char*>& requestedExtensions,
		const bool debug
	);

	//QueueFamily


	void create_logical_device(VkPhysicalDevice& physicalDevice, VkDevice& device, VkSurfaceKHR& surface, bool debugMode);
	void get_Queues(VkPhysicalDevice& physicalDevice, VkDevice& device, VkQueue queues[4],VkSurfaceKHR& surface, bool debugMode);


}