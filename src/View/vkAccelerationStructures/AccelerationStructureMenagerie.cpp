#include "View/vkAccelerationStructures/AccelerationStructureMenagerie.h"
#include "View/vkAccelerationStructures/AccelerationStructure.h"
#include <View/vkUtil/memory.h>
#include <View/vkInit/commands.h>












vkAccelerationStructure::VertexMenagerie::VertexMenagerie() {
	 vertexLump = {
		    1.0f,  1.0f, 0.0f ,
		   -1.0f,  1.0f, 0.0f ,
		    0.0f, -1.0f, 0.0f
	};

	// Setup indices
	indexLump = { 0, 1, 2 };
	//indexCount = static_cast<uint32_t>(indices.size());

	
}

vkAccelerationStructure::VertexMenagerie::~VertexMenagerie() {

}

void vkAccelerationStructure::VertexMenagerie::consume(uint64_t meshType, std::vector<float> data, std::vector<uint32_t> indices) {



}

void vkAccelerationStructure::VertexMenagerie::finalize(vkAccelerationStructure::FinalizationChunk finalizationChunk, VkCommandPool commandPool) {

	logicalDevice = finalizationChunk.logicalDevice;



	// Setup identity transform matrix
	VkTransformMatrixKHR transformMatrix = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f
	};
	//make a staging buffer for vertices
	BufferInputChunk inputChunk{};
	inputChunk.logicalDevice = finalizationChunk.logicalDevice;
	inputChunk.physicalDevice = finalizationChunk.physicalDevice;
	inputChunk.size = sizeof(float) * vertexLump.size();
	inputChunk.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		
	inputChunk.memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	Buffer stagingBuffer;

	vkUtil::createBuffer(inputChunk, stagingBuffer);
	
	inputChunk.memoryAllocatet = VkMemoryAllocateFlagBits::VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
	//fill it with vertex data
	void* memoryLocation = nullptr;
	vkMapMemory(logicalDevice, stagingBuffer.bufferMemory, 0, inputChunk.size, 0, &memoryLocation);
	memcpy(memoryLocation, vertexLump.data(), inputChunk.size);
	vkUnmapMemory(logicalDevice, stagingBuffer.bufferMemory);

	inputChunk.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
	inputChunk.memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	
	vkUtil::createBuffer(inputChunk, vertexBuffer);

	//copy to it
	vkUtil::copyBuffer(
		stagingBuffer, vertexBuffer, inputChunk.size,
		finalizationChunk.queue, finalizationChunk.commandBuffer
	);
	
	// Cleanup
	vkDestroyBuffer(logicalDevice, stagingBuffer.buffer, nullptr);
	vkFreeMemory(logicalDevice, stagingBuffer.bufferMemory, nullptr);


	// Staging buffer for indices
	inputChunk.size = sizeof(uint32_t) * indexLump.size();
	inputChunk.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	inputChunk.memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	
	vkUtil::createBuffer(inputChunk, stagingBuffer);


	vkMapMemory(logicalDevice, stagingBuffer.bufferMemory, 0, inputChunk.size, 0, &memoryLocation);
	memcpy(memoryLocation, indexLump.data(), inputChunk.size);
	vkUnmapMemory(logicalDevice, stagingBuffer.bufferMemory);

	// Create index buffer
	inputChunk.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
	inputChunk.memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	

	vkUtil::createBuffer(inputChunk, indexBuffer);

	// Copy data to index buffer
	vkUtil::copyBuffer(stagingBuffer, indexBuffer, inputChunk.size, finalizationChunk.queue, finalizationChunk.commandBuffer);

	// Cleanup
	vkDestroyBuffer(logicalDevice, stagingBuffer.buffer, nullptr);
	vkFreeMemory(logicalDevice, stagingBuffer.bufferMemory, nullptr);


	// Staging buffer for indices
	inputChunk.size = sizeof(VkTransformMatrixKHR);
	inputChunk.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	inputChunk.memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	vkUtil::createBuffer(inputChunk, stagingBuffer);


	vkMapMemory(logicalDevice, stagingBuffer.bufferMemory, 0, inputChunk.size, 0, &memoryLocation);
	memcpy(memoryLocation, &transformMatrix, inputChunk.size);
	vkUnmapMemory(logicalDevice, stagingBuffer.bufferMemory);

	// Create index buffer
	inputChunk.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
	inputChunk.memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	

	vkUtil::createBuffer(inputChunk, transformBuffer);

	// Copy data to index buffer
	vkUtil::copyBuffer(stagingBuffer, transformBuffer, inputChunk.size, finalizationChunk.queue, finalizationChunk.commandBuffer);

	// Cleanup
	vkDestroyBuffer(logicalDevice, stagingBuffer.buffer, nullptr);
	vkFreeMemory(logicalDevice, stagingBuffer.bufferMemory, nullptr);


	VkDeviceOrHostAddressConstKHR vertexBufferDeviceAddress{};
	VkDeviceOrHostAddressConstKHR indexBufferDeviceAddress{};
	VkDeviceOrHostAddressConstKHR transformBufferDeviceAddress{};

	vertexBufferDeviceAddress.deviceAddress = vkUtil::getBufferDeviceAddress(logicalDevice,vertexBuffer.buffer);
	indexBufferDeviceAddress.deviceAddress = vkUtil::getBufferDeviceAddress(logicalDevice, indexBuffer.buffer);
	transformBufferDeviceAddress.deviceAddress = vkUtil::getBufferDeviceAddress(logicalDevice, transformBuffer.buffer);


	// Build
	VkAccelerationStructureGeometryKHR accelerationStructureGeometry{};
	accelerationStructureGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
	accelerationStructureGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
	accelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
	accelerationStructureGeometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
	accelerationStructureGeometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
	accelerationStructureGeometry.geometry.triangles.vertexData = vertexBufferDeviceAddress;
	accelerationStructureGeometry.geometry.triangles.maxVertex = 2;
	accelerationStructureGeometry.geometry.triangles.vertexStride = sizeof(float)* vertexLump.size();
	accelerationStructureGeometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
	accelerationStructureGeometry.geometry.triangles.indexData = indexBufferDeviceAddress;
	accelerationStructureGeometry.geometry.triangles.transformData.deviceAddress = 0;
	accelerationStructureGeometry.geometry.triangles.transformData.hostAddress = nullptr;
	accelerationStructureGeometry.geometry.triangles.transformData = transformBufferDeviceAddress;

	// Get size info
	VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfo{};
	accelerationStructureBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	accelerationStructureBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
	accelerationStructureBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
	accelerationStructureBuildGeometryInfo.geometryCount = 1;
	accelerationStructureBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;

	PFN_vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizesKHR;
	vkGetAccelerationStructureBuildSizesKHR = (PFN_vkGetAccelerationStructureBuildSizesKHR)(vkGetDeviceProcAddr(logicalDevice, "vkGetAccelerationStructureBuildSizesKHR"));
	
	PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR;
	vkCreateAccelerationStructureKHR = (PFN_vkCreateAccelerationStructureKHR)(vkGetDeviceProcAddr(logicalDevice, "vkCreateAccelerationStructureKHR"));

	const uint32_t numTriangles = 1;
	VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfo{};
	accelerationStructureBuildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
	vkGetAccelerationStructureBuildSizesKHR(
		logicalDevice,
		VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
		&accelerationStructureBuildGeometryInfo,
		&numTriangles,
		&accelerationStructureBuildSizesInfo);

	vkAccelerationStructure::createAccelerationStructureBuffer(finalizationChunk.physicalDevice,finalizationChunk.logicalDevice,bottomLevelAS, accelerationStructureBuildSizesInfo);

	VkAccelerationStructureCreateInfoKHR accelerationStructureCreateInfo{};
	accelerationStructureCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
	accelerationStructureCreateInfo.buffer = bottomLevelAS.buffer;
	accelerationStructureCreateInfo.size = accelerationStructureBuildSizesInfo.accelerationStructureSize;
	accelerationStructureCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
	vkCreateAccelerationStructureKHR(logicalDevice, &accelerationStructureCreateInfo, nullptr, &bottomLevelAS.handle);

	// Create a small scratch buffer used during build of the bottom level acceleration structure
	vkAccelerationStructure::RayTracingScratchBuffer scratchBuffer = vkAccelerationStructure::createScratchBuffer(finalizationChunk.physicalDevice, finalizationChunk.logicalDevice, accelerationStructureBuildSizesInfo.buildScratchSize);

	VkAccelerationStructureBuildGeometryInfoKHR accelerationBuildGeometryInfo{};
	accelerationBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	accelerationBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
	accelerationBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
	accelerationBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
	accelerationBuildGeometryInfo.dstAccelerationStructure = bottomLevelAS.handle;
	accelerationBuildGeometryInfo.geometryCount = 1;
	accelerationBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;
	accelerationBuildGeometryInfo.scratchData.deviceAddress = scratchBuffer.deviceAddress;

	VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo{};
	accelerationStructureBuildRangeInfo.primitiveCount = numTriangles;
	accelerationStructureBuildRangeInfo.primitiveOffset = 0;
	accelerationStructureBuildRangeInfo.firstVertex = 0;
	accelerationStructureBuildRangeInfo.transformOffset = 0;
	std::vector<VkAccelerationStructureBuildRangeInfoKHR*> accelerationBuildStructureRangeInfos = { &accelerationStructureBuildRangeInfo };

	// Build the acceleration structure on the device via a one-time command buffer submission
	// Some implementations may support acceleration structure building on the host (VkPhysicalDeviceAccelerationStructureFeaturesKHR->accelerationStructureHostCommands), but we prefer device builds
	
	PFN_vkCmdBuildAccelerationStructuresKHR vkCmdBuildAccelerationStructuresKHR;
	vkCmdBuildAccelerationStructuresKHR = (PFN_vkCmdBuildAccelerationStructuresKHR)(vkGetDeviceProcAddr(logicalDevice, "vkCmdBuildAccelerationStructuresKHR"));

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // Mo¿esz zmieniæ flagi, jeœli potrzeba

	VkResult result = vkBeginCommandBuffer(finalizationChunk.commandBuffer, &beginInfo);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to begin recording command buffer!");
	}

	vkCmdBuildAccelerationStructuresKHR(
		finalizationChunk.commandBuffer,
		1,
		&accelerationBuildGeometryInfo,
		accelerationBuildStructureRangeInfos.data());

	if (vkEndCommandBuffer(finalizationChunk.commandBuffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to record command buffer!");
	}

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &finalizationChunk.commandBuffer;

	//vkGetDeviceQueue(device, selectedQueueFamilyIndex, 0, &graphicsQueue);
	// Wys³anie komendy kopiowania na kolejkê
	if (vkQueueSubmit(finalizationChunk.queue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit copy command buffer!");
	}

	// Oczekiwanie na zakoñczenie operacji
	vkQueueWaitIdle(finalizationChunk.queue);

	PFN_vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddressKHR;
	vkGetAccelerationStructureDeviceAddressKHR = (PFN_vkGetAccelerationStructureDeviceAddressKHR)(vkGetDeviceProcAddr(logicalDevice, "vkGetAccelerationStructureDeviceAddressKHR"));

	VkAccelerationStructureDeviceAddressInfoKHR accelerationDeviceAddressInfo{};
	accelerationDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
	accelerationDeviceAddressInfo.accelerationStructure = bottomLevelAS.handle;
	bottomLevelAS.deviceAddress = vkGetAccelerationStructureDeviceAddressKHR(logicalDevice, &accelerationDeviceAddressInfo);

	deleteScratchBuffer(finalizationChunk.logicalDevice,scratchBuffer);
	create_top_acceleration_structure(finalizationChunk.physicalDevice, finalizationChunk.commandBuffer, finalizationChunk.queue, commandPool);
}

void vkAccelerationStructure::VertexMenagerie::create_top_acceleration_structure(VkPhysicalDevice physicalDevice, VkCommandBuffer commandBuffer, VkQueue queue, VkCommandPool commandPool) {
	VkTransformMatrixKHR transformMatrix = {
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f };

	PFN_vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizesKHR;
	vkGetAccelerationStructureBuildSizesKHR = (PFN_vkGetAccelerationStructureBuildSizesKHR)(vkGetDeviceProcAddr(logicalDevice, "vkGetAccelerationStructureBuildSizesKHR"));

	PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR;
	vkCreateAccelerationStructureKHR = (PFN_vkCreateAccelerationStructureKHR)(vkGetDeviceProcAddr(logicalDevice, "vkCreateAccelerationStructureKHR"));
	PFN_vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddressKHR;
	vkGetAccelerationStructureDeviceAddressKHR = (PFN_vkGetAccelerationStructureDeviceAddressKHR)(vkGetDeviceProcAddr(logicalDevice, "vkGetAccelerationStructureDeviceAddressKHR"));

	VkAccelerationStructureInstanceKHR instance{};
	instance.transform = transformMatrix;
	instance.instanceCustomIndex = 0;
	instance.mask = 0xFF;
	instance.instanceShaderBindingTableRecordOffset = 0;
	instance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
	instance.accelerationStructureReference = bottomLevelAS.deviceAddress;

	// Buffer for instance data
	Buffer instanceBuffer;
	BufferInputChunk inputChunk{};
	inputChunk.logicalDevice = logicalDevice;
	inputChunk.physicalDevice = physicalDevice;
	inputChunk.size = sizeof(VkAccelerationStructureInstanceKHR);
	inputChunk.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	inputChunk.memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	
	Buffer stagingBuffer;
	
	vkUtil::createBuffer(inputChunk, stagingBuffer);

	inputChunk.memoryAllocatet = VkMemoryAllocateFlagBits::VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
	//fill it with vertex data
	void* memoryLocation = nullptr;
	vkMapMemory(logicalDevice, stagingBuffer.bufferMemory, 0, inputChunk.size, 0, &memoryLocation);
	memcpy(memoryLocation, &instance, inputChunk.size);
	vkUnmapMemory(logicalDevice, stagingBuffer.bufferMemory);

	inputChunk.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
	inputChunk.memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	
	vkUtil::createBuffer(inputChunk, instanceBuffer);

	//copy to it
	vkUtil::copyBuffer(
		stagingBuffer, instanceBuffer, inputChunk.size,
		queue, commandBuffer
	);

	// Cleanup
	vkDestroyBuffer(logicalDevice, stagingBuffer.buffer, nullptr);
	vkFreeMemory(logicalDevice, stagingBuffer.bufferMemory, nullptr);




	VkDeviceOrHostAddressConstKHR instanceDataDeviceAddress{};
	instanceDataDeviceAddress.deviceAddress = vkUtil::getBufferDeviceAddress(logicalDevice,instanceBuffer.buffer);

	VkAccelerationStructureGeometryKHR accelerationStructureGeometry{};
	accelerationStructureGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
	accelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
	accelerationStructureGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
	accelerationStructureGeometry.geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
	accelerationStructureGeometry.geometry.instances.arrayOfPointers = VK_FALSE;
	accelerationStructureGeometry.geometry.instances.data = instanceDataDeviceAddress;

	// Get size info
	/*
	The pSrcAccelerationStructure, dstAccelerationStructure, and mode members of pBuildInfo are ignored. Any VkDeviceOrHostAddressKHR members of pBuildInfo are ignored by this command, except that the hostAddress member of VkAccelerationStructureGeometryTrianglesDataKHR::transformData will be examined to check if it is NULL.*
	*/
	VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfo{};
	accelerationStructureBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	accelerationStructureBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
	accelerationStructureBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
	accelerationStructureBuildGeometryInfo.geometryCount = 1;
	accelerationStructureBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;

	uint32_t primitive_count = 1;

	VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfo{};
	accelerationStructureBuildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
	vkGetAccelerationStructureBuildSizesKHR(
		logicalDevice,
		VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
		&accelerationStructureBuildGeometryInfo,
		&primitive_count,
		&accelerationStructureBuildSizesInfo);

	vkAccelerationStructure::createAccelerationStructureBuffer(physicalDevice,logicalDevice,topLevelAS, accelerationStructureBuildSizesInfo);

	VkAccelerationStructureCreateInfoKHR accelerationStructureCreateInfo{};
	accelerationStructureCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
	accelerationStructureCreateInfo.buffer = topLevelAS.buffer;
	accelerationStructureCreateInfo.size = accelerationStructureBuildSizesInfo.accelerationStructureSize;
	accelerationStructureCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
	vkCreateAccelerationStructureKHR(logicalDevice, &accelerationStructureCreateInfo, nullptr, &topLevelAS.handle);

	// Create a small scratch buffer used during build of the top level acceleration structure
	vkAccelerationStructure::RayTracingScratchBuffer scratchBuffer = vkAccelerationStructure::createScratchBuffer(physicalDevice,logicalDevice,accelerationStructureBuildSizesInfo.buildScratchSize);

	VkAccelerationStructureBuildGeometryInfoKHR accelerationBuildGeometryInfo{};
	accelerationBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	accelerationBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
	accelerationBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
	accelerationBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
	accelerationBuildGeometryInfo.dstAccelerationStructure = topLevelAS.handle;
	accelerationBuildGeometryInfo.geometryCount = 1;
	accelerationBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;
	accelerationBuildGeometryInfo.scratchData.deviceAddress = scratchBuffer.deviceAddress;

	VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo{};
	accelerationStructureBuildRangeInfo.primitiveCount = 1;
	accelerationStructureBuildRangeInfo.primitiveOffset = 0;
	accelerationStructureBuildRangeInfo.firstVertex = 0;
	accelerationStructureBuildRangeInfo.transformOffset = 0;
	std::vector<VkAccelerationStructureBuildRangeInfoKHR*> accelerationBuildStructureRangeInfos = { &accelerationStructureBuildRangeInfo };

	// Build the acceleration structure on the device via a one-time command buffer submission
	// Some implementations may support acceleration structure building on the host (VkPhysicalDeviceAccelerationStructureFeaturesKHR->accelerationStructureHostCommands), but we prefer device builds
	PFN_vkCmdBuildAccelerationStructuresKHR vkCmdBuildAccelerationStructuresKHR;
	vkCmdBuildAccelerationStructuresKHR = (PFN_vkCmdBuildAccelerationStructuresKHR)(vkGetDeviceProcAddr(logicalDevice, "vkCmdBuildAccelerationStructuresKHR"));
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // Mo¿esz zmieniæ flagi, jeœli potrzeba

	VkResult result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to begin recording command buffer!");
	}
	vkCmdBuildAccelerationStructuresKHR(
		commandBuffer,
		1,
		&accelerationBuildGeometryInfo,
		accelerationBuildStructureRangeInfos.data());
	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to record command buffer!");
	}

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	//vkGetDeviceQueue(device, selectedQueueFamilyIndex, 0, &graphicsQueue);
	// Wys³anie komendy kopiowania na kolejkê
	if (vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit copy command buffer!");
	}

	// Oczekiwanie na zakoñczenie operacji
	vkQueueWaitIdle(queue);

	VkAccelerationStructureDeviceAddressInfoKHR accelerationDeviceAddressInfo{};
	accelerationDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
	accelerationDeviceAddressInfo.accelerationStructure = topLevelAS.handle;
	topLevelAS.deviceAddress = vkGetAccelerationStructureDeviceAddressKHR(logicalDevice, &accelerationDeviceAddressInfo);

	deleteScratchBuffer(logicalDevice,scratchBuffer);
	
}
