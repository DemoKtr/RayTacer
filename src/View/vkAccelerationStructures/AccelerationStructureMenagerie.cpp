#include "View/vkAccelerationStructures/AccelerationStructureMenagerie.h"
#include "View/vkAccelerationStructures/AccelerationStructure.h"
#include <View/vkUtil/memory.h>
#include <View/vkInit/commands.h>

#include "glm/gtc/type_ptr.hpp"
#include "View/vkMesh/obj_mesh.h"

vkAccelerationStructure::VertexMenagerie::VertexMenagerie() {}

vkAccelerationStructure::VertexMenagerie::~VertexMenagerie() {}




void vkAccelerationStructure::VertexMenagerie::create_blas(vkAccelerationStructure::FinalizationChunk finalizationChunk,vkMesh::ObjMesh mesh,VkTransformMatrixKHR transformMatrix) {
    Buffer vertexBuffer, indexBuffer,uvBuffer, transformBuffer;
    bottomLevelASes.push_back(vkAccelerationStructure::AccelerationStructure {});
    transformMatrixes.push_back(transformMatrix);


    extraBLASoffsets.push_back( mesh.v.size());
    for (const auto& v :  mesh.v)
    {
        inputArray.push_back(v.x);
        inputArray.push_back(v.y);
        inputArray.push_back(v.z);
    }
    totalExtraBLASBufferSize += mesh.v.size() * sizeof(glm::vec3);

    
    extraBLASoffsets.push_back( mesh.indices.size());
    for (const auto& indice :  mesh.indices)
    {
        inputArray.push_back(static_cast<float>(indice));
    }
    totalExtraBLASBufferSize += mesh.indices.size() * sizeof(glm::vec3);

    extraBLASoffsets.push_back( mesh.normals.size());
    for (const auto& normal :  mesh.normals)
    {
        inputArray.push_back(normal);
    }
    totalExtraBLASBufferSize += mesh.normals.size() * sizeof(glm::vec3);

    extraBLASoffsets.push_back( mesh.uv.size());
    for (const auto& uv :  mesh.uv)
    {
        inputArray.push_back(uv);
    }
    totalExtraBLASBufferSize += mesh.uv.size() * sizeof(glm::vec2);
    
    size += sizeof(float) * mesh.indices.size();
    
    const uint32_t  numTriangles = mesh.indices.size() / 3;
    
    logicalDevice = finalizationChunk.logicalDevice;
    
    //make a staging buffer for vertices
    BufferInputChunk inputChunk{};
    inputChunk.logicalDevice = finalizationChunk.logicalDevice;
    inputChunk.physicalDevice = finalizationChunk.physicalDevice;
    inputChunk.size = sizeof(float) * mesh.vertices.size();
    inputChunk.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    inputChunk.memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    Buffer stagingBuffer;

    vkUtil::createBuffer(inputChunk, stagingBuffer);

    inputChunk.memoryAllocatet = VkMemoryAllocateFlagBits::VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    //fill it with vertex data
    void *memoryLocation = nullptr;
    vkMapMemory(logicalDevice, stagingBuffer.bufferMemory, 0, inputChunk.size, 0, &memoryLocation);
    memcpy(memoryLocation, mesh.vertices.data(), inputChunk.size);
    vkUnmapMemory(logicalDevice, stagingBuffer.bufferMemory);

    inputChunk.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
                       VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
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
    inputChunk.size = sizeof(uint32_t) * mesh.indices.size();
    inputChunk.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    inputChunk.memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    vkUtil::createBuffer(inputChunk, stagingBuffer);


    vkMapMemory(logicalDevice, stagingBuffer.bufferMemory, 0, inputChunk.size, 0, &memoryLocation);
    memcpy(memoryLocation, mesh.indices.data(), inputChunk.size);
    vkUnmapMemory(logicalDevice, stagingBuffer.bufferMemory);

    // Create index buffer
    inputChunk.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
                       VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
    inputChunk.memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;


    vkUtil::createBuffer(inputChunk, indexBuffer);

    // Copy data to index buffer
    vkUtil::copyBuffer(stagingBuffer, indexBuffer, inputChunk.size, finalizationChunk.queue,
                       finalizationChunk.commandBuffer);

    // Cleanup
    vkDestroyBuffer(logicalDevice, stagingBuffer.buffer, nullptr);
    vkFreeMemory(logicalDevice, stagingBuffer.bufferMemory, nullptr);


    // Staging buffer for VkTransformMatrixKHR
    inputChunk.size = sizeof(VkTransformMatrixKHR);
    inputChunk.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    inputChunk.memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    vkUtil::createBuffer(inputChunk, stagingBuffer);


    vkMapMemory(logicalDevice, stagingBuffer.bufferMemory, 0, inputChunk.size, 0, &memoryLocation);
    memcpy(memoryLocation, &transformMatrix, inputChunk.size);
    vkUnmapMemory(logicalDevice, stagingBuffer.bufferMemory);

    // Create index buffer
    inputChunk.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
                       VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
    inputChunk.memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;


    vkUtil::createBuffer(inputChunk, transformBuffer);

    // Copy data to index buffer
    vkUtil::copyBuffer(stagingBuffer, transformBuffer, inputChunk.size, finalizationChunk.queue,
                       finalizationChunk.commandBuffer);

    // Cleanup
    vkDestroyBuffer(logicalDevice, stagingBuffer.buffer, nullptr);
    vkFreeMemory(logicalDevice, stagingBuffer.bufferMemory, nullptr);


    VkDeviceOrHostAddressConstKHR vertexBufferDeviceAddress{};
    VkDeviceOrHostAddressConstKHR indexBufferDeviceAddress{};
    VkDeviceOrHostAddressConstKHR transformBufferDeviceAddress{};

    vertexBufferDeviceAddress.deviceAddress = vkUtil::getBufferDeviceAddress(logicalDevice, vertexBuffer.buffer);
    indexBufferDeviceAddress.deviceAddress = vkUtil::getBufferDeviceAddress(logicalDevice, indexBuffer.buffer);
    transformBufferDeviceAddress.deviceAddress = vkUtil::getBufferDeviceAddress(logicalDevice, transformBuffer.buffer);


    // Build
    VkAccelerationStructureGeometryKHR accelerationStructureGeometry{};
    accelerationStructureGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
    accelerationStructureGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
    accelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
    accelerationStructureGeometry.geometry.triangles.sType =
            VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
    accelerationStructureGeometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
    accelerationStructureGeometry.geometry.triangles.vertexData = vertexBufferDeviceAddress;
    accelerationStructureGeometry.geometry.triangles.maxVertex = *std::ranges::max_element(mesh.indices);
    accelerationStructureGeometry.geometry.triangles.vertexStride = sizeof(float) * 3;
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
    vkGetAccelerationStructureBuildSizesKHR = (PFN_vkGetAccelerationStructureBuildSizesKHR) (vkGetDeviceProcAddr(
        logicalDevice, "vkGetAccelerationStructureBuildSizesKHR"));

    PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR;
    vkCreateAccelerationStructureKHR = (PFN_vkCreateAccelerationStructureKHR) (vkGetDeviceProcAddr(
        logicalDevice, "vkCreateAccelerationStructureKHR"));

    VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfo{};
    accelerationStructureBuildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
    vkGetAccelerationStructureBuildSizesKHR(
        logicalDevice,
        VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
        &accelerationStructureBuildGeometryInfo,
        &numTriangles,
        &accelerationStructureBuildSizesInfo);

    vkAccelerationStructure::createAccelerationStructureBuffer(finalizationChunk.physicalDevice,
                                                               finalizationChunk.logicalDevice, bottomLevelASes.back(),
                                                               accelerationStructureBuildSizesInfo);

    VkAccelerationStructureCreateInfoKHR accelerationStructureCreateInfo{};
    accelerationStructureCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
    accelerationStructureCreateInfo.buffer = bottomLevelASes.back().buffer;
    accelerationStructureCreateInfo.size = accelerationStructureBuildSizesInfo.accelerationStructureSize;
    accelerationStructureCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    vkCreateAccelerationStructureKHR(logicalDevice, &accelerationStructureCreateInfo, nullptr, &bottomLevelASes.back().handle);

    vkAccelerationStructure::RayTracingScratchBuffer scratchBuffer = vkAccelerationStructure::createScratchBuffer(
        finalizationChunk.physicalDevice, finalizationChunk.logicalDevice,
        accelerationStructureBuildSizesInfo.buildScratchSize);

    VkAccelerationStructureBuildGeometryInfoKHR accelerationBuildGeometryInfo{};
    accelerationBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
    accelerationBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    accelerationBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
    accelerationBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    accelerationBuildGeometryInfo.dstAccelerationStructure = bottomLevelASes.back().handle;
    accelerationBuildGeometryInfo.geometryCount = 1;
    accelerationBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;
    accelerationBuildGeometryInfo.scratchData.deviceAddress = scratchBuffer.deviceAddress;

    VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo{};
    accelerationStructureBuildRangeInfo.primitiveCount = numTriangles;
    accelerationStructureBuildRangeInfo.primitiveOffset = 0;
    accelerationStructureBuildRangeInfo.firstVertex = 0;
    accelerationStructureBuildRangeInfo.transformOffset = 0;
    std::vector<VkAccelerationStructureBuildRangeInfoKHR *> accelerationBuildStructureRangeInfos = {
        &accelerationStructureBuildRangeInfo
    };

    // Build the acceleration structure on the device via a one-time command buffer submission
    // Some implementations may support acceleration structure building on the host (VkPhysicalDeviceAccelerationStructureFeaturesKHR->accelerationStructureHostCommands), but we prefer device builds

    PFN_vkCmdBuildAccelerationStructuresKHR vkCmdBuildAccelerationStructuresKHR;
    vkCmdBuildAccelerationStructuresKHR = (PFN_vkCmdBuildAccelerationStructuresKHR) (vkGetDeviceProcAddr(
        logicalDevice, "vkCmdBuildAccelerationStructuresKHR"));

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // Mo�esz zmieni� flagi, je�li potrzeba

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
    // Wys�anie komendy kopiowania na kolejk�
    if (vkQueueSubmit(finalizationChunk.queue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit copy command buffer!");
    }

    // Oczekiwanie na zako�czenie operacji
    vkQueueWaitIdle(finalizationChunk.queue);

    PFN_vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddressKHR;
    vkGetAccelerationStructureDeviceAddressKHR = (PFN_vkGetAccelerationStructureDeviceAddressKHR) (vkGetDeviceProcAddr(
        logicalDevice, "vkGetAccelerationStructureDeviceAddressKHR"));

    VkAccelerationStructureDeviceAddressInfoKHR accelerationDeviceAddressInfo{};
    accelerationDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
    accelerationDeviceAddressInfo.accelerationStructure = bottomLevelASes.back().handle;
    bottomLevelASes.back().deviceAddress = vkGetAccelerationStructureDeviceAddressKHR(
        logicalDevice, &accelerationDeviceAddressInfo);
    
    deleteScratchBuffer(finalizationChunk.logicalDevice, scratchBuffer);
}

void vkAccelerationStructure::VertexMenagerie::finalize(vkAccelerationStructure::FinalizationChunk finalizationChunk,
                                                        VkCommandPool commandPool, uint32_t &re) {
    re = size;
    create_top_acceleration_structure(finalizationChunk.physicalDevice, finalizationChunk.commandBuffer,finalizationChunk.queue, commandPool);
}
void vkAccelerationStructure::VertexMenagerie::create_top_acceleration_structure(
    VkPhysicalDevice physicalDevice, VkCommandBuffer commandBuffer, VkQueue queue, VkCommandPool commandPool) {

    PFN_vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizesKHR;
    vkGetAccelerationStructureBuildSizesKHR = (PFN_vkGetAccelerationStructureBuildSizesKHR) (vkGetDeviceProcAddr(
        logicalDevice, "vkGetAccelerationStructureBuildSizesKHR"));

    PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR;
    vkCreateAccelerationStructureKHR = (PFN_vkCreateAccelerationStructureKHR) (vkGetDeviceProcAddr(
        logicalDevice, "vkCreateAccelerationStructureKHR"));
    PFN_vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddressKHR;
    vkGetAccelerationStructureDeviceAddressKHR = (PFN_vkGetAccelerationStructureDeviceAddressKHR) (vkGetDeviceProcAddr(
        logicalDevice, "vkGetAccelerationStructureDeviceAddressKHR"));

    std::vector<VkAccelerationStructureInstanceKHR> instances;
    instances.reserve(bottomLevelASes.size());
    
    // Populate each instance with a reference to a different BLAS
    for (uint32_t i = 0; i < bottomLevelASes.size(); ++i) {
        instances.push_back(VkAccelerationStructureInstanceKHR{});
        instances[i].transform = transformMatrixes[i];  // Assuming a shared transform matrix
        instances[i].instanceCustomIndex = i;
        instances[i].mask = 0xFF;
        instances[i].instanceShaderBindingTableRecordOffset = 0;
        instances[i].flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
        
        // Assign the device address of the corresponding BLAS
        instances[i].accelerationStructureReference = bottomLevelASes[i].deviceAddress;  // Assuming an array of BLAS

    }

    // Buffer for instance data
    Buffer instanceBuffer;
    BufferInputChunk inputChunk{};
    inputChunk.logicalDevice = logicalDevice;
    inputChunk.physicalDevice = physicalDevice;
    inputChunk.size = sizeof(VkAccelerationStructureInstanceKHR) * bottomLevelASes.size();  // Size for multiple instances
    inputChunk.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    inputChunk.memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    Buffer stagingBuffer;

    vkUtil::createBuffer(inputChunk, stagingBuffer);

    inputChunk.memoryAllocatet = VkMemoryAllocateFlagBits::VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    // Fill it with vertex data (instance data)
    void *memoryLocation = nullptr;
    vkMapMemory(logicalDevice, stagingBuffer.bufferMemory, 0, inputChunk.size, 0, &memoryLocation);
    memcpy(memoryLocation, instances.data(), inputChunk.size);  // Copy the instances array
    vkUnmapMemory(logicalDevice, stagingBuffer.bufferMemory);

    inputChunk.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                       VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                       VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    inputChunk.memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    vkUtil::createBuffer(inputChunk, instanceBuffer);

    // Copy to it
    vkUtil::copyBuffer(
        stagingBuffer, instanceBuffer, inputChunk.size,
        queue, commandBuffer
    );

    // Cleanup
    vkDestroyBuffer(logicalDevice, stagingBuffer.buffer, nullptr);
    vkFreeMemory(logicalDevice, stagingBuffer.bufferMemory, nullptr);

    VkDeviceOrHostAddressConstKHR instanceDataDeviceAddress{};
    instanceDataDeviceAddress.deviceAddress = vkUtil::getBufferDeviceAddress(logicalDevice, instanceBuffer.buffer);

    VkAccelerationStructureGeometryKHR accelerationStructureGeometry{};
    accelerationStructureGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
    accelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
    accelerationStructureGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
    accelerationStructureGeometry.geometry.instances.sType =
            VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
    accelerationStructureGeometry.geometry.instances.arrayOfPointers = VK_FALSE;
    accelerationStructureGeometry.geometry.instances.data = instanceDataDeviceAddress;

    // Get size info
    VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfo{};
    accelerationStructureBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
    accelerationStructureBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
    accelerationStructureBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
    accelerationStructureBuildGeometryInfo.geometryCount = 1;
    accelerationStructureBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;

    uint32_t primitive_count = bottomLevelASes.size();  // Number of instances

    VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfo{};
    accelerationStructureBuildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
    vkGetAccelerationStructureBuildSizesKHR(
        logicalDevice,
        VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
        &accelerationStructureBuildGeometryInfo,
        &primitive_count,
        &accelerationStructureBuildSizesInfo);

    vkAccelerationStructure::createAccelerationStructureBuffer(physicalDevice, logicalDevice, topLevelAS,
                                                               accelerationStructureBuildSizesInfo);

    VkAccelerationStructureCreateInfoKHR accelerationStructureCreateInfo{};
    accelerationStructureCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
    accelerationStructureCreateInfo.buffer = topLevelAS.buffer;
    accelerationStructureCreateInfo.size = accelerationStructureBuildSizesInfo.accelerationStructureSize;
    accelerationStructureCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
    vkCreateAccelerationStructureKHR(logicalDevice, &accelerationStructureCreateInfo, nullptr, &topLevelAS.handle);

    // Create a small scratch buffer used during build of the top level acceleration structure
    vkAccelerationStructure::RayTracingScratchBuffer scratchBuffer = vkAccelerationStructure::createScratchBuffer(
        physicalDevice, logicalDevice, accelerationStructureBuildSizesInfo.buildScratchSize);

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
    accelerationStructureBuildRangeInfo.primitiveCount = bottomLevelASes.size();  // Adjusted for multiple instances
    accelerationStructureBuildRangeInfo.primitiveOffset = 0;
    accelerationStructureBuildRangeInfo.firstVertex = 0;
    accelerationStructureBuildRangeInfo.transformOffset = 0;
    std::vector<VkAccelerationStructureBuildRangeInfoKHR *> accelerationBuildStructureRangeInfos = {
        &accelerationStructureBuildRangeInfo
    };

    // Build the acceleration structure on the device via a one-time command buffer submission
    PFN_vkCmdBuildAccelerationStructuresKHR vkCmdBuildAccelerationStructuresKHR;
    vkCmdBuildAccelerationStructuresKHR = (PFN_vkCmdBuildAccelerationStructuresKHR) (vkGetDeviceProcAddr(
        logicalDevice, "vkCmdBuildAccelerationStructuresKHR"));
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

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

    if (vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit copy command buffer!");
    }

    // Oczekiwanie na zakończenie operacji
    vkQueueWaitIdle(queue);

    VkAccelerationStructureDeviceAddressInfoKHR accelerationDeviceAddressInfo{};
    accelerationDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
    accelerationDeviceAddressInfo.accelerationStructure = topLevelAS.handle;
    topLevelAS.deviceAddress =
            vkGetAccelerationStructureDeviceAddressKHR(logicalDevice, &accelerationDeviceAddressInfo);

    deleteScratchBuffer(logicalDevice, scratchBuffer);
}

