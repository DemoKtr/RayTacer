#include "View/vkInit/logging.h"

VKAPI_ATTR VkBool32 VKAPI_CALL vkInit::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
}

VkDebugUtilsMessengerEXT vkInit::make_debug_messenger(VkInstance& instance)
{
    VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;

    // Wype³nienie struktury tworzenia debug messengera
    VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = nullptr;

    // Pobranie wskaŸnika do funkcji vkCreateDebugUtilsMessengerEXT
    PFN_vkCreateDebugUtilsMessengerEXT fpCreateDebugMessenger =
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (fpCreateDebugMessenger != nullptr) {
        if (fpCreateDebugMessenger(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
            std::cerr << "Failed to create debug messenger!" << std::endl;
        }
    }
    else {
        std::cerr << "Could not load vkCreateDebugUtilsMessengerEXT" << std::endl;
    }

    return debugMessenger;
}

void vkInit::log_device_properties(const VkPhysicalDevice& device) {
	VkPhysicalDeviceProperties properties;
	vkGetPhysicalDeviceProperties(device, &properties);

	/*
	 typedef struct VkPhysicalDeviceProperties {
		uint32_t                            apiVersion;
		uint32_t                            driverVersion;
		uint32_t                            vendorID;
		uint32_t                            deviceID;
		VkPhysicalDeviceType                deviceType;
		char                                deviceName[VK_MAX_PHYSICAL_DEVICE_NAME_SIZE];
		uint8_t                             pipelineCacheUUID[VK_UUID_SIZE];
		VkPhysicalDeviceLimits              limits;
		VkPhysicalDeviceSparseProperties    sparseProperties;
		} VkPhysicalDeviceProperties;
	*/

	std::cout << "Device name: " << properties.deviceName << '\n';

	std::cout << "Device type: ";
	switch (properties.deviceType) {

	case (VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_CPU):
		std::cout << "CPU\n";
		break;

	case (VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU):
		std::cout << "Discrete GPU\n";
		break;

	case (VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU):
		std::cout << "Integrated GPU\n";
		break;

	case (VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU):
		std::cout << "Virtual GPU\n";
		break;

	default:
		std::cout << "Other\n";
	}

}

std::vector<std::string> vkInit::log_transform_bits(VkSurfaceTransformFlagsKHR bits)
{
	std::vector<std::string> result;

	/*
		* typedef enum VkSurfaceTransformFlagBitsKHR {
			VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR = 0x00000001,
			VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR = 0x00000002,
			VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR = 0x00000004,
			VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR = 0x00000008,
			VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_BIT_KHR = 0x00000010,
			VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_90_BIT_KHR = 0x00000020,
			VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_180_BIT_KHR = 0x00000040,
			VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_270_BIT_KHR = 0x00000080,
			VK_SURFACE_TRANSFORM_INHERIT_BIT_KHR = 0x00000100,
		} VkSurfaceTransformFlagBitsKHR;
	*/
	if (bits & VkSurfaceTransformFlagBitsKHR::VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
		result.push_back("identity");
	}
	if (bits & VkSurfaceTransformFlagBitsKHR::VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_90_BIT_KHR) {
		result.push_back("90 degree rotation");
	}
	if (bits & VkSurfaceTransformFlagBitsKHR::VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_180_BIT_KHR) {
		result.push_back("180 degree rotation");
	}
	if (bits & VkSurfaceTransformFlagBitsKHR::VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_270_BIT_KHR) {
		result.push_back("270 degree rotation");
	}
	if (bits & VkSurfaceTransformFlagBitsKHR::VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_BIT_KHR) {
		result.push_back("horizontal mirror");
	}
	if (bits & VkSurfaceTransformFlagBitsKHR::VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_90_BIT_KHR) {
		result.push_back("horizontal mirror, then 90 degree rotation");
	}
	if (bits & VkSurfaceTransformFlagBitsKHR::VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_180_BIT_KHR) {
		result.push_back("horizontal mirror, then 180 degree rotation");
	}
	if (bits & VkSurfaceTransformFlagBitsKHR::VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_270_BIT_KHR) {
		result.push_back("horizontal mirror, then 270 degree rotation");
	}
	if (bits & VkSurfaceTransformFlagBitsKHR::VK_SURFACE_TRANSFORM_INHERIT_BIT_KHR) {
		result.push_back("inherited");
	}

	return result;
}

std::vector<std::string> vkInit::log_alpha_composite_bits(VkCompositeAlphaFlagsKHR bits)
{
	std::vector<std::string> result;

	/*
		typedef enum VkCompositeAlphaFlagBitsKHR {
			VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR = 0x00000001,
			VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR = 0x00000002,
			VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR = 0x00000004,
			VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR = 0x00000008,
		} VkCompositeAlphaFlagBitsKHR;
	*/
	if (bits & VkCompositeAlphaFlagBitsKHR::VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR) {
		result.push_back("opaque (alpha ignored)");
	}
	if (bits & VkCompositeAlphaFlagBitsKHR::VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR) {
		result.push_back("pre multiplied (alpha expected to already be multiplied in image)");
	}
	if (bits & VkCompositeAlphaFlagBitsKHR::VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR) {
		result.push_back("post multiplied (alpha will be applied during composition)");
	}
	if (bits & VkCompositeAlphaFlagBitsKHR::VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR) {
		result.push_back("inherited");
	}

	return result;
}

std::vector<std::string> vkInit::log_image_usage_bits(VkImageUsageFlags bits)
{
	std::vector<std::string> result;

	/*
		typedef enum VkImageUsageFlagBits {
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT = 0x00000001,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT = 0x00000002,
			VK_IMAGE_USAGE_SAMPLED_BIT = 0x00000004,
			VK_IMAGE_USAGE_STORAGE_BIT = 0x00000008,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT = 0x00000010,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT = 0x00000020,
			VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT = 0x00000040,
			VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT = 0x00000080,
			#ifdef VK_ENABLE_BETA_EXTENSIONS
				// Provided by VK_KHR_video_decode_queue
				VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR = 0x00000400,
			#endif
			#ifdef VK_ENABLE_BETA_EXTENSIONS
				// Provided by VK_KHR_video_decode_queue
				VK_IMAGE_USAGE_VIDEO_DECODE_SRC_BIT_KHR = 0x00000800,
			#endif
			#ifdef VK_ENABLE_BETA_EXTENSIONS
				// Provided by VK_KHR_video_decode_queue
				VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR = 0x00001000,
			#endif
			// Provided by VK_EXT_fragment_density_map
			VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT = 0x00000200,
			// Provided by VK_KHR_fragment_shading_rate
			VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR = 0x00000100,
			#ifdef VK_ENABLE_BETA_EXTENSIONS
				// Provided by VK_KHR_video_encode_queue
				VK_IMAGE_USAGE_VIDEO_ENCODE_DST_BIT_KHR = 0x00002000,
			#endif
			#ifdef VK_ENABLE_BETA_EXTENSIONS
				// Provided by VK_KHR_video_encode_queue
				VK_IMAGE_USAGE_VIDEO_ENCODE_SRC_BIT_KHR = 0x00004000,
			#endif
			#ifdef VK_ENABLE_BETA_EXTENSIONS
				// Provided by VK_KHR_video_encode_queue
				VK_IMAGE_USAGE_VIDEO_ENCODE_DPB_BIT_KHR = 0x00008000,
			#endif
			// Provided by VK_HUAWEI_invocation_mask
			VK_IMAGE_USAGE_INVOCATION_MASK_BIT_HUAWEI = 0x00040000,
			// Provided by VK_NV_shading_rate_image
			VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV = VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR,
	} VkImageUsageFlagBits;
	*/
	/**/
	if (bits & VkImageUsageFlagBits::VK_IMAGE_USAGE_HOST_TRANSFER_BIT_EXT) {
		result.push_back("transfer src: image can be used as the source of a transfer command.");
	}
	if (bits & VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_DST_BIT) {
		result.push_back("transfer dst: image can be used as the destination of a transfer command.");
	}
	if (bits & VkImageUsageFlagBits::VK_IMAGE_USAGE_SAMPLED_BIT) {
		result.push_back("sampled: image can be used to create a VkImageView suitable for occupying a \
VkDescriptorSet slot either of type VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE or \
VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, and be sampled by a shader.");
	}
	if (bits & VkImageUsageFlagBits::VK_IMAGE_USAGE_STORAGE_BIT) {
		result.push_back("storage: image can be used to create a VkImageView suitable for occupying a \
VkDescriptorSet slot of type VK_DESCRIPTOR_TYPE_STORAGE_IMAGE.");
	}
	if (bits & VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) {
		result.push_back("color attachment: image can be used to create a VkImageView suitable for use as \
a color or resolve attachment in a VkFramebuffer.");
	}
	if (bits & VkImageUsageFlagBits::VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
		result.push_back("depth/stencil attachment: image can be used to create a VkImageView \
suitable for use as a depth/stencil or depth/stencil resolve attachment in a VkFramebuffer.");
	}
	if (bits & VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT) {
		result.push_back("transient attachment: implementations may support using memory allocations \
with the VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT to back an image with this usage. This \
bit can be set for any image that can be used to create a VkImageView suitable for use as \
a color, resolve, depth/stencil, or input attachment.");
	}
	if (bits & VkImageUsageFlagBits::VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT) {
		result.push_back("input attachment: image can be used to create a VkImageView suitable for \
occupying VkDescriptorSet slot of type VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT; be read from \
a shader as an input attachment; and be used as an input attachment in a framebuffer.");
	}
	if (bits & VkImageUsageFlagBits::VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT) {
		result.push_back("fragment density map: image can be used to create a VkImageView suitable \
for use as a fragment density map image.");
	}
	if (bits & VkImageUsageFlagBits::VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR) {
		result.push_back("fragment shading rate attachment: image can be used to create a VkImageView \
suitable for use as a fragment shading rate attachment or shading rate image");
	}
	return result;
}

std::string vkInit::log_present_mode(VkPresentModeKHR presentMode)
{
	/*
		* // Provided by VK_KHR_surface
		typedef enum VkPresentModeKHR {
			VK_PRESENT_MODE_IMMEDIATE_KHR = 0,
			VK_PRESENT_MODE_MAILBOX_KHR = 1,
			VK_PRESENT_MODE_FIFO_KHR = 2,
			VK_PRESENT_MODE_FIFO_RELAXED_KHR = 3,
			// Provided by VK_KHR_shared_presentable_image
			VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR = 1000111000,
			// Provided by VK_KHR_shared_presentable_image
			VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR = 1000111001,
		} VkPresentModeKHR;
		*/

	if (presentMode == VkPresentModeKHR::VK_PRESENT_MODE_IMMEDIATE_KHR) {
		return "immediate: the presentation engine does not wait for a vertical blanking period \
to update the current image, meaning this mode may result in visible tearing. No internal \
queuing of presentation requests is needed, as the requests are applied immediately.";
	}
	if (presentMode == VkPresentModeKHR::VK_PRESENT_MODE_MAILBOX_KHR) {
		return "mailbox: the presentation engine waits for the next vertical blanking period \
to update the current image. Tearing cannot be observed. An internal single-entry queue is \
used to hold pending presentation requests. If the queue is full when a new presentation \
request is received, the new request replaces the existing entry, and any images associated \
with the prior entry become available for re-use by the application. One request is removed \
from the queue and processed during each vertical blanking period in which the queue is non-empty.";
	}
	if (presentMode == VkPresentModeKHR::VK_PRESENT_MODE_FIFO_KHR) {
		return "fifo: the presentation engine waits for the next vertical blanking \
period to update the current image. Tearing cannot be observed. An internal queue is used to \
hold pending presentation requests. New requests are appended to the end of the queue, and one \
request is removed from the beginning of the queue and processed during each vertical blanking \
period in which the queue is non-empty. This is the only value of presentMode that is required \
to be supported.";
	}
	if (presentMode == VkPresentModeKHR::VK_PRESENT_MODE_FIFO_RELAXED_KHR) {
		return "relaxed fifo: the presentation engine generally waits for the next vertical \
blanking period to update the current image. If a vertical blanking period has already passed \
since the last update of the current image then the presentation engine does not wait for \
another vertical blanking period for the update, meaning this mode may result in visible tearing \
in this case. This mode is useful for reducing visual stutter with an application that will \
mostly present a new image before the next vertical blanking period, but may occasionally be \
late, and present a new image just after the next vertical blanking period. An internal queue \
is used to hold pending presentation requests. New requests are appended to the end of the queue, \
and one request is removed from the beginning of the queue and processed during or after each \
vertical blanking period in which the queue is non-empty.";
	}
	if (presentMode == VkPresentModeKHR::VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR) {
		return "shared demand refresh: the presentation engine and application have \
concurrent access to a single image, which is referred to as a shared presentable image. \
The presentation engine is only required to update the current image after a new presentation \
request is received. Therefore the application must make a presentation request whenever an \
update is required. However, the presentation engine may update the current image at any point, \
meaning this mode may result in visible tearing.";
	}
	if (presentMode == VkPresentModeKHR::VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR) {
		return "shared continuous refresh: the presentation engine and application have \
concurrent access to a single image, which is referred to as a shared presentable image. The \
presentation engine periodically updates the current image on its regular refresh cycle. The \
application is only required to make one initial presentation request, after which the \
presentation engine must update the current image without any need for further presentation \
requests. The application can indicate the image contents have been updated by making a \
presentation request, but this does not guarantee the timing of when it will be updated. \
This mode may result in visible tearing if rendering to the image is not timed correctly.";
	}
	return "none/undefined";
}

