#pragma once
#include "config.h"
#include <vector>
namespace vkInit {
	VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData
	);

	/**
		Make a debug messenger

		\param instance The Vulkan instance which will be debugged.
		\param dldi dynamically loads instance based dispatch functions
		\returns the created messenger
	*/

	VkDebugUtilsMessengerEXT make_debug_messenger(VkInstance& instance);
	void log_device_properties(const VkPhysicalDevice& device);

	std::vector<std::string> log_transform_bits(VkSurfaceTransformFlagsKHR bits);
	std::vector<std::string> log_alpha_composite_bits(VkCompositeAlphaFlagsKHR bits);
	std::vector<std::string> log_image_usage_bits(VkImageUsageFlags bits);
	std::string log_present_mode(VkPresentModeKHR presentMode);


}