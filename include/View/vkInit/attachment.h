#pragma once
#include "config.h"

namespace VkInit {

	void create_attachment(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VkFormat f, VkImageUsageFlagBits u, VkExtent2D swapchainExten, VkImage& image, VkDeviceMemory& mem, VkImageView& view);

}