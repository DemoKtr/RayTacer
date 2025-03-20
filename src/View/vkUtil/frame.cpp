#include "View/vkUtil/frame.h"

void vkUtil::SwapChainFrame::destroy() {
	vkDestroyImageView(logicalDevice, mainImageView, nullptr);

	vkDestroySemaphore(logicalDevice, renderFinished, nullptr);
	vkDestroySemaphore(logicalDevice, imageAvailable, nullptr);
	vkDestroySemaphore(logicalDevice, computeFinished, nullptr);
	vkDestroyFence(logicalDevice, inFlight, nullptr);
}

void vkUtil::SwapChainFrame::make_descriptors_resources() {
}
