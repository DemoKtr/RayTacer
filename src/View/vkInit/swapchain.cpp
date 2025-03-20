#include "View/vkInit/swapchain.h"
#include <View/vkInit/logging.h>
#include <View/vkUtil/queues.h>
#include <View/vkImage/Image.h>

vkInit::SwapChainSupportDetails vkInit::query_swapchain_support(VkPhysicalDevice device, VkSurfaceKHR surface, bool debugMode)
{
	SwapChainSupportDetails support;
	/*
* typedef struct VkSurfaceCapabilitiesKHR {
	uint32_t                         minImageCount;
	uint32_t                         maxImageCount;
	VkExtent2D                       currentExtent;
	VkExtent2D                       minImageExtent;
	VkExtent2D                       maxImageExtent;
	uint32_t                         maxImageArrayLayers;
	VkSurfaceTransformFlagsKHR       supportedTransforms;
	VkSurfaceTransformFlagBitsKHR    currentTransform;
	VkCompositeAlphaFlagsKHR         supportedCompositeAlpha;
	VkImageUsageFlags                supportedUsageFlags;
} VkSurfaceCapabilitiesKHR;
*/
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &surfaceCapabilities);
	support.capabilities = surfaceCapabilities;
	if (debugMode) {
		std::cout << "Swapchain can support the following surface capabilities:" << std::endl;

		std::cout << "tminimum image count: " << support.capabilities.minImageCount << std::endl;
		std::cout << "tmaximum image count: " << support.capabilities.maxImageCount << std::endl;

		std::cout << "\tcurrent extent: \n";
		/*typedef struct VkExtent2D {
			uint32_t    width;
			uint32_t    height;
		} VkExtent2D;
		*/
		std::cout << "\t\twidth: " << support.capabilities.currentExtent.width << std::endl;
		std::cout << "\t\theight: " << support.capabilities.currentExtent.height << std::endl;

		std::cout << "\tminimum supported extent: " << std::endl;
		std::cout << "\t\twidth: " << support.capabilities.minImageExtent.width << std::endl;
		std::cout << "\t\theight: " << support.capabilities.minImageExtent.height << std::endl;

		std::cout << "\tmaximum supported extent: " << std::endl;
		std::cout << "\t\twidth: " << support.capabilities.maxImageExtent.width << std::endl;
		std::cout << "\t\theight: " << support.capabilities.maxImageExtent.height << std::endl;

		std::cout << "\tmaximum image array layers: " << support.capabilities.maxImageArrayLayers << std::endl;

		std::cout << "\tsupported transforms: " << std::endl;;
		std::vector<std::string> stringList = log_transform_bits(support.capabilities.supportedTransforms);
		for (std::string line : stringList) {
			std::cout << "\t\t" << line << std::endl;;
		}

		std::cout << "\tcurrent transform:\n";
		stringList = log_transform_bits(support.capabilities.currentTransform);
		for (std::string line : stringList) {
			std::cout << "\t\t" << line << std::endl;
		}

		std::cout << "\tsupported alpha operations:" << std::endl;;
		stringList = log_alpha_composite_bits(support.capabilities.supportedCompositeAlpha);
		for (std::string line : stringList) {
			std::cout << "\t\t" << line << std::endl;;
		}

		std::cout << "\tsupported image usage:" << std::endl;
		stringList = log_image_usage_bits(support.capabilities.supportedUsageFlags);
		for (std::string line : stringList) {
			std::cout << "\t\t" << line << std::endl;
		}
	}

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

	std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, surfaceFormats.data());

	support.formats = surfaceFormats;

	/*
	if (debugMode) {

		for (VkSurfaceFormatKHR supportedFormat : support.formats) {
			
			 typedef struct VkSurfaceFormatKHR {
				VkFormat           format;
				VkColorSpaceKHR    colorSpace;
			} VkSurfaceFormatKHR;
			
			
			//std::cout << "supported pixel format: " << Vkto_string(supportedFormat.format) << '\n';
			//std::cout << "supported color space: " << Vk_string(supportedFormat.colorSpace) << '\n';
		}
	}
	*/
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

	std::vector<VkPresentModeKHR> presentModes(presentModeCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, presentModes.data());

	support.presentModes = presentModes;

	for (VkPresentModeKHR presentMode : support.presentModes) {
		std::cout << '\t' << log_present_mode(presentMode) << '\n';
	}

	return support;

}

VkSurfaceFormatKHR vkInit::choose_swapchain_surface_format(std::vector<VkSurfaceFormatKHR> formats)
{
	for (VkSurfaceFormatKHR format : formats) {
		if (format.format == VK_FORMAT_R8G8B8A8_UNORM && format.colorSpace == VkColorSpaceKHR::VK_COLORSPACE_SRGB_NONLINEAR_KHR) {
			return format;
		}
		
	}
}

VkPresentModeKHR vkInit::choose_swapchain_present_mode(std::vector<VkPresentModeKHR> presentModes)
{
	for (VkPresentModeKHR presentMode : presentModes) {
		if (presentMode == VkPresentModeKHR::VK_PRESENT_MODE_MAILBOX_KHR) {
			return presentMode;
		}
	}
	return VkPresentModeKHR::VK_PRESENT_MODE_MAILBOX_KHR;
}

VkExtent2D vkInit::choose_swapchain_exten(uint32_t width, uint32_t height, VkSurfaceCapabilitiesKHR capabilities)
{
	if (capabilities.currentExtent.width != UINT32_MAX) {
		return capabilities.currentExtent;
	}
	else {
		VkExtent2D extent = { width,height };
		extent.width = std::min(
			capabilities.maxImageExtent.width,
			std::max(capabilities.minImageExtent.width, width)
		);
		extent.height = std::min(
			capabilities.maxImageExtent.height,
			std::max(capabilities.minImageExtent.height, height)
		);
		return extent;
	}
}

vkInit::SwapChainBundle vkInit::create_swapchain(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VkSurfaceKHR surface, int width, int height, bool debugMode) {
	SwapChainBundle bundle{};

	// Pobranie informacji o swapchainie
	SwapChainSupportDetails support = query_swapchain_support(physicalDevice, surface, debugMode);
	VkSurfaceFormatKHR format = choose_swapchain_surface_format(support.formats);
	VkPresentModeKHR presentMode = choose_swapchain_present_mode(support.presentModes);
	VkExtent2D extent = choose_swapchain_exten(width, height, support.capabilities);

	// Ustalanie liczby obrazów w swapchainie
	uint32_t imageCount = support.capabilities.minImageCount + 1;
	if (support.capabilities.maxImageCount > 0 && imageCount > support.capabilities.maxImageCount) {
		imageCount = support.capabilities.maxImageCount;
	}

	// Konfiguracja struktury tworzenia swapchaina
	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = format.format;
	createInfo.imageColorSpace = format.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	// Znalezienie rodzin kolejek
	vkUtil::QueueFamilyIndices indices = vkUtil::findQueueFamilies(physicalDevice, surface, debugMode);
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	// Ustawienie trybu wspó³dzielenia obrazu miêdzy kolejkami
	if (indices.graphicsFamily.value() != indices.presentFamily.value()) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}

	// Pozosta³e ustawienia swapchaina
	createInfo.preTransform = support.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	// Tworzenie swapchaina
	if (vkCreateSwapchainKHR(logicalDevice, &createInfo, nullptr, &bundle.swapchain) != VK_SUCCESS) {
		throw std::runtime_error("failed to create swap chain!");
	}

	// Pobranie obrazów ze swapchaina
	uint32_t swapchainImageCount = 0;
	vkGetSwapchainImagesKHR(logicalDevice, bundle.swapchain, &swapchainImageCount, nullptr);
	std::vector<VkImage> images(swapchainImageCount);
	vkGetSwapchainImagesKHR(logicalDevice, bundle.swapchain, &swapchainImageCount, images.data());

	bundle.frames.resize(images.size());

	for (size_t i = 0; i < images.size(); i++) {
		bundle.frames[i].mainImage = images[i];

		  vkImage::make_image_view(
			logicalDevice, images[i], bundle.frames[i].mainImageView,format.format, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_2D, 1
		);

		
	}

	bundle.format = format.format;
	bundle.extent = extent;

	return bundle;

}
