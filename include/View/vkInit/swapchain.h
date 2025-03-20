#pragma once
#include "config.h"
#include "View/vkUtil/frame.h"
#include <vector>


namespace vkInit
{
	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities; ///ile obrazow w swapchain
		std::vector<VkSurfaceFormatKHR> formats; //pixelformat
		std::vector<VkPresentModeKHR> presentModes; // wybiera jak dobieraæ obrazy
	};

	struct SwapChainBundle {
		VkSwapchainKHR swapchain;
		std::vector<vkUtil::SwapChainFrame> frames;
		VkFormat format;
		VkExtent2D extent;
	};



	SwapChainSupportDetails query_swapchain_support(VkPhysicalDevice device, VkSurfaceKHR surface, bool debugMode);
	VkSurfaceFormatKHR choose_swapchain_surface_format(std::vector<VkSurfaceFormatKHR> formats);
	VkPresentModeKHR choose_swapchain_present_mode(std::vector<VkPresentModeKHR> presentModes);
	VkExtent2D choose_swapchain_exten(uint32_t width, uint32_t height, VkSurfaceCapabilitiesKHR capabilities);
	SwapChainBundle create_swapchain(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VkSurfaceKHR surface, int width, int height, bool debugMode);

};