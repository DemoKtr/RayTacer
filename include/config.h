#pragma once
#include <vulkan/vulkan.h>
#include <iostream>
#include <optional>
#include <fstream>

struct Buffer {
	VkBuffer buffer;
	VkDeviceMemory bufferMemory;
};

struct BufferInputChunk {
	size_t size;
	VkBufferUsageFlags usage;
	VkDevice logicalDevice;
	VkPhysicalDevice physicalDevice;
	VkMemoryPropertyFlags memoryProperties;
	VkMemoryAllocateFlags memoryAllocatet;
	
};

