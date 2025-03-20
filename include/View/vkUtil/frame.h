#pragma once
#include "config.h"
#include "descriptorsBuffers.h"

namespace vkUtil {

	class SwapChainFrame {
	public:
		VkDevice device;
		VkImage mainImage;
		VkImageView mainImageView;
		void destroy();
	};

}
