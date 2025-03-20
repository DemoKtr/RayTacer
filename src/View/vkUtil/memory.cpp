#include "View/vkUtil/memory.h"

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
	/*
// Provided by VK_VERSION_1_0
typedef struct VkMemoryRequirements {
	VkDeviceSize    size;
	VkDeviceSize    alignment;
	uint32_t        memoryTypeBits;
} VkMemoryRequirements;
*/
	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(input.logicalDevice, buffer.buffer, &memoryRequirements);

	/*
	* // Provided by VK_VERSION_1_0
	typedef struct VkMemoryAllocateInfo {
		VkStructureType    sType;
		const void*        pNext;
		VkDeviceSize       allocationSize;
		uint32_t           memoryTypeIndex;
	} VkMemoryAllocateInfo;
	*/
	VkMemoryAllocateInfo allocInfo;
	allocInfo.allocationSize = memoryRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryTypeIndex(
		input.physicalDevice, memoryRequirements.memoryTypeBits,
		input.memoryProperties
	);
	if (!(input.memoryAllocatet == VkMemoryAllocateFlags())) {
		VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo;
		memoryAllocateFlagsInfo.flags = input.memoryAllocatet;

		// Jeœli u¿ywasz `bufferDeviceAddress`, dodaj odpowiedni¹ flagê
		allocInfo.pNext = &memoryAllocateFlagsInfo;
	}

	vkAllocateMemory(input.logicalDevice, &allocInfo, nullptr, &buffer.bufferMemory);
	
	vkBindBufferMemory(input.logicalDevice, buffer.buffer, buffer.bufferMemory, 0);
}

void vkUtil::createBuffer(BufferInputChunk input)
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
	bufferInfo.flags = VkBufferCreateFlags();
	bufferInfo.size = input.size;
	bufferInfo.usage = input.usage;
	bufferInfo.sharingMode = VkSharingMode::VK_SHARING_MODE_EXCLUSIVE;

	
	vkCreateBuffer(input.logicalDevice, &bufferInfo, nullptr, &input.buffer.buffer);

	allocateBufferMemory(input.buffer, input);
	
}

void vkUtil::copyBuffer(Buffer& srcBuffer, Buffer& dstBuffer, VkDeviceSize size, VkQueue queue, VkCommandBuffer commandBuffer) {
    // Resetowanie bufora poleceñ przed u¿yciem
    vkResetCommandBuffer(commandBuffer, 0);

    // Struktura begin info dla command buffer
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // Jednorazowe u¿ycie

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    // Konfiguracja operacji kopiowania bufora
    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0; // Pocz¹tek Ÿród³a
    copyRegion.dstOffset = 0; // Pocz¹tek celu
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

    // Wys³anie komendy kopiowania na kolejkê
    if (vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit copy command buffer!");
    }

    // Oczekiwanie na zakoñczenie operacji
    vkQueueWaitIdle(queue);
}
