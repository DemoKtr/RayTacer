#pragma once
#include "config.h"

namespace vkUtil {
	void startJob(VkCommandBuffer commandBuffer);
	void endJob(VkCommandBuffer, VkQueue queue);
}