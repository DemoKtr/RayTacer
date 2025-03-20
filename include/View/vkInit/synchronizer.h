#pragma once
#include "config.h"

namespace vkInit {


	void make_semaphore(VkDevice device, VkSemaphore& semaphore,bool debug);

	/**
		Make a fence.

		\param device the logical device
		\param debug whether the system is running in debug mode
		\returns the created fence
	*/
	void make_fence(VkDevice device,VkFence& fence ,bool debug);
}