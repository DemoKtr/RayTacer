#include "View/vkUtil/frame.h"

void vkUtil::SwapChainFrame::destroy() {
	vkDestroyImageView(device, mainImageView, nullptr);
}
