#pragma once
#include "config.h"
#include <optional>

#include <vector>
namespace vkUtil {
	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily; //moze nie musi zawierac warttosci .has_value sprawdza
		std::optional<uint32_t> presentFamily;
		std::optional<uint32_t> computeFamily;
		std::optional<uint32_t> transferFamily;
		bool isComplete() {
			return (graphicsFamily.has_value() && presentFamily.has_value() && computeFamily.has_value() && transferFamily.has_value()

				);
		}
	};

	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, bool debugMode);
}