#include "View/vkUtil/memory.h"

#include <string>
#include <vector>

uint32_t vkUtil::findMemoryTypeIndex(VkPhysicalDevice physicalDevice, uint32_t supportedMemoryIndices, VkMemoryPropertyFlags requestedProperties) {
	/*
* // Provided by VK_VERSION_1_0
typedef struct VkPhysicalDeviceMemoryProperties {
	uint32_t        memoryTypeCount;
	VkMemoryType    memoryTypes[VK_MAX_MEMORY_TYPES];
	uint32_t        memoryHeapCount;
	VkMemoryHeap    memoryHeaps[VK_MAX_MEMORY_HEAPS];
} VkPhysicalDeviceMemoryProperties;
*/
	VkPhysicalDeviceMemoryProperties memoryProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {

		//bit i of supportedMemoryIndices is set if that memory type is supported by the device
		bool supported{ static_cast<bool>(supportedMemoryIndices & (1 << i)) };

		//propertyFlags holds all the memory properties supported by this memory type
		bool sufficient{ (memoryProperties.memoryTypes[i].propertyFlags & requestedProperties) == requestedProperties };

		if (supported && sufficient) {
			return i;
		}
	}

	return 0;
}

void vkUtil::allocateBufferMemory(Buffer& buffer, const BufferInputChunk& input) {
	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(input.logicalDevice, buffer.buffer, &memoryRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memoryRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryTypeIndex(
		input.physicalDevice, memoryRequirements.memoryTypeBits, input.memoryProperties
	);

	// Sprawdzenie, czy trzeba dodać VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT
	VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo{};

		memoryAllocateFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
		memoryAllocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
		allocInfo.pNext = &memoryAllocateFlagsInfo;


	// Alokacja pamięci
	VkResult result = vkAllocateMemory(input.logicalDevice, &allocInfo, nullptr, &buffer.bufferMemory);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate buffer memory!" + std::to_string(result));
	}

	// Przypisanie pamięci do bufora
	if (vkBindBufferMemory(input.logicalDevice, buffer.buffer, buffer.bufferMemory, 0) != VK_SUCCESS) {
		throw std::runtime_error("Failed to bind buffer memory!");
	}

	std::cout << "Memory allocated successfully for buffer: " << buffer.bufferMemory << std::endl;
}

void vkUtil::createBuffer(BufferInputChunk input, Buffer& buffer)
{
	/*
* // Provided by VK_VERSION_1_0
typedef struct VkBufferCreateInfo {
	VkStructureType        sType;
	const void*            pNext;
	VkBufferCreateFlags    flags;
	VkDeviceSize           size;
	VkBufferUsageFlags     usage;
	VkSharingMode          sharingMode;
	uint32_t               queueFamilyIndexCount;
	const uint32_t*        pQueueFamilyIndices;
} VkBufferCreateInfo;
*/
	VkBufferCreateInfo bufferInfo;
	bufferInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.flags = VkBufferCreateFlags();
	bufferInfo.size = input.size;
	bufferInfo.usage = input.usage;
	bufferInfo.sharingMode = VkSharingMode::VK_SHARING_MODE_EXCLUSIVE;
	bufferInfo.pNext = nullptr;
	

	
 	vkCreateBuffer(input.logicalDevice, &bufferInfo, nullptr, &buffer.buffer);

	allocateBufferMemory(buffer, input);
	
}

void vkUtil::copyBuffer(Buffer& srcBuffer, Buffer& dstBuffer, VkDeviceSize size, VkQueue queue, VkCommandBuffer commandBuffer) {
    // Resetowanie bufora poleceń przed użyciem
    vkResetCommandBuffer(commandBuffer, 0);

    // Struktura begin info dla command buffer
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // Jednorazowe użycie

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    // Konfiguracja operacji kopiowania bufora
    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0; // Początek źródła
    copyRegion.dstOffset = 0; // Początek celu
    copyRegion.size = size;   // Rozmiar kopiowania

    vkCmdCopyBuffer(commandBuffer, srcBuffer.buffer, dstBuffer.buffer, 1, &copyRegion);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }

    // Struktura dla submit
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

	//vkGetDeviceQueue(device, selectedQueueFamilyIndex, 0, &graphicsQueue);
    // Wysłanie komendy kopiowania na kolejkę
    if (vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit copy command buffer!");
    }

    // Oczekiwanie na zakończenie operacji
    vkQueueWaitIdle(queue);
}

uint64_t vkUtil::getBufferDeviceAddress(VkDevice device, VkBuffer buffer) {
	
	PFN_vkGetBufferDeviceAddressKHR vkGetBufferDeviceAddressKHR = (PFN_vkGetBufferDeviceAddressKHR)vkGetDeviceProcAddr(device, "vkGetBufferDeviceAddressKHR");
	VkBufferDeviceAddressInfoKHR bufferDeviceAI{};
	bufferDeviceAI.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
	bufferDeviceAI.buffer = buffer;
	return vkGetBufferDeviceAddressKHR(device, &bufferDeviceAI);
}

uint32_t vkUtil::getMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32* memTypeFound, VkPhysicalDeviceMemoryProperties& memoryProperties) {

	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
	{
		if ((typeBits & 1) == 1)
		{
			if ((memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				if (memTypeFound)
				{
					*memTypeFound = true;
				}
				return i;
			}
		}
		typeBits >>= 1;
	}

	if (memTypeFound)
	{
		*memTypeFound = false;
		return 0;
	}
	else
	{
		throw std::runtime_error("Could not find a matching memory type");
	}
}



