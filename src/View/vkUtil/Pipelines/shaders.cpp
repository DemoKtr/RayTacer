#include "View/vkUtil/Pipelines/shaders.h"

std::vector<char> vkUtil::readFile(std::string filename) {

	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		std::cout << "Failed to load \"" << filename << std::endl;

	}

	size_t filesize{ static_cast<size_t>(file.tellg()) };

	std::vector<char> buffer(filesize);
	file.seekg(0);
	file.read(buffer.data(), filesize);

	file.close();
	return buffer;
}

void vkUtil::createModule(std::string filename, VkDevice device, VkShaderModule& shaderModule) {

	std::vector<char> sourceCode = vkUtil::readFile(filename);

	VkShaderModuleCreateInfo moduleInfo = {};
	moduleInfo.flags = VkShaderModuleCreateFlags();
	moduleInfo.codeSize = sourceCode.size();
	moduleInfo.pCode = reinterpret_cast<const uint32_t*>(sourceCode.data());

	
	VkResult result = vkCreateShaderModule(device, &moduleInfo, nullptr, &shaderModule);

	if (result != VK_SUCCESS) {
		std::cerr << "Failed to create shader module for \"" << filename << "\", error code: " << result << std::endl;
		
	}

	
}
