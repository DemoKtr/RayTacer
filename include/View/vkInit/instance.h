#pragma once
#include "config.h"
#include "vulkan/vulkan.h"
#include <vector>
namespace vkInit {

	bool supported(std::vector<const char*>& extensions, std::vector<const char*>& layers, bool debug);
	void make_instance(VkInstance& instance,bool debugMode, const char* appName);

}