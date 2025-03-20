#pragma once
#include "config.h"
#include "descriptorsBuffers.h"

namespace vkUtil {

	class SwapChainFrame {
	public:
		VkDevice logicalDevice;
		VkPhysicalDevice physicalDevice;
		VkSemaphore imageAvailable, renderFinished, computeFinished;
		VkFence inFlight;
		VkImage mainImage;
		VkImageView mainImageView;





		VkCommandBuffer mainCommandBuffer;
		VkCommandBuffer computeCommandBuffer;
		void destroy();
		void make_descriptors_resources();
	};

}
