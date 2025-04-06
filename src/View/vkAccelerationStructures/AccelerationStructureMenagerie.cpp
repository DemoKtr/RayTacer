#include "View/vkAccelerationStructures/AccelerationStructureMenagerie.h"
#include "View/vkAccelerationStructures/AccelerationStructure.h"
#include <View/vkUtil/memory.h>
#include <View/vkInit/commands.h>

#include "glm/gtc/type_ptr.hpp"

vkAccelerationStructure::VertexMenagerie::VertexMenagerie() {
    
    transformMatrix = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f
    };
    /*
    vertexLump = {
        1.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f,
        0.0f, -1.0f, 0.0f
    };

    // Setup indices
    indexLump = {0, 1, 2};
    //indexCount = static_cast<uint32_t>(indices.size());
    */
}

vkAccelerationStructure::VertexMenagerie::~VertexMenagerie() {}


void vkAccelerationStructure::VertexMenagerie::consume(PrefabType prefabType, std::vector<float> data,
                                                       std::vector<uint32_t> indices) {

    int indexCount = static_cast<int>(indices.size());
    int vertexCount = static_cast<int>(data.size() / 3);
    VertexCount = static_cast<int>(data.size() / 3);
    
    int lastIndex = static_cast<int>(indexLump.size());

    firstIndices.insert(std::make_pair(prefabType, lastIndex));
    indexCounts.insert(std::make_pair(prefabType, indexCount));
    for (float attribute : data) {
        vertexLump.push_back(attribute);
    }
    for (uint32_t index : indices) {
        indexLump.push_back(index + indexOffset);
    }
    numTriangles = indexLump.size()/3;
    indexOffset += vertexCount;


}

/*
void vkAccelerationStructure::VertexMenagerie::consume(PrefabType prefabType) {
    switch (prefabType) {
        default:
            throw std::runtime_error("No type to consume has been assigned!");
        case PrefabType::CUBE:
            vertexLump = {
                -1.0f, -1.0f, -1.0f,
                1.0f, -1.0f, -1.0f,
                1.0f, 1.0f, -1.0f,
                -1.0f, 1.0f, -1.0f,
                -1.0f, -1.0f, 1.0f,
                1.0f, -1.0f, 1.0f,
                1.0f, 1.0f, 1.0f,
                -1.0f, 1.0f, 1.0f
            };

        // Setup indices
            indexLump = {
                0, 1, 2, 2, 3, 0, // Front face
                4, 5, 6, 6, 7, 4, // Back face
                1, 5, 6, 6, 2, 1, // Right face
                0, 4, 7, 7, 3, 0, // Left face
                3, 2, 6, 6, 7, 3, // Top face
                0, 1, 5, 5, 4, 0 // Bottom face
            };

        numTriangles = 12;
            break;
        case PrefabType::PLANE:
            vertexLump = {
                -1.0f, -1.0f, 0.0f, // Vertex 0: Bottom left corner
                1.0f, -1.0f, 0.0f, // Vertex 1: Bottom right corner
                1.0f, 1.0f, 0.0f, // Vertex 2: Top right corner
                -1.0f, 1.0f, 0.0f // Vertex 3: Top left corner
            };

        // Setup indices
            indexLump = {0, 1, 2, 2, 3, 0};

        numTriangles = 2;
            break;
        case PrefabType::SPHERE: {
            int sectors = 10; // horizontal slices
            int stacks = 10; // vertical slices

            numTriangles = 2 * sectors * (stacks - 2) + 2 * sectors;
            
            float PI = acos(-1);

            vertexLump.clear();
            indexLump.clear();

            for (int i = 0; i <= stacks; ++i) {
                float V = i / (float) stacks;
                float phi = V * PI;

                for (int j = 0; j <= sectors; ++j) {
                    float U = j / (float) sectors;
                    float theta = U * (PI * 2);

                    float x = cos(theta) * sin(phi);
                    float y = cos(phi);
                    float z = sin(theta) * sin(phi);

                    vertexLump.push_back(x);
                    vertexLump.push_back(y);
                    vertexLump.push_back(z);
                }
            }

            for (int i = 0; i < stacks; ++i) {
                int k1 = i * (sectors + 1);
                int k2 = k1 + sectors + 1;

                for (int j = 0; j < sectors; ++j, ++k1, ++k2) {
                    if (i != 0) {
                        indexLump.push_back(k1);
                        indexLump.push_back(k2);
                        indexLump.push_back(k1 + 1);
                    }

                    if (i != (stacks - 1)) {
                        indexLump.push_back(k1 + 1);
                        indexLump.push_back(k2);
                        indexLump.push_back(k2 + 1);
                    }
                }
            }
        }
        break;
    }
}
*/
void vkAccelerationStructure::VertexMenagerie::transform(glm::vec3 vector) {
    transformMatrix.matrix[0][3] += vector[0];
    transformMatrix.matrix[1][3] += vector[1];
    transformMatrix.matrix[2][3] += vector[2];
}


void vkAccelerationStructure::VertexMenagerie::finalize(vkAccelerationStructure::FinalizationChunk finalizationChunk,
                                                        VkCommandPool commandPool, uint32_t &re) {
    logicalDevice = finalizationChunk.logicalDevice;

    //make a staging buffer for vertices
    BufferInputChunk inputChunk{};
    inputChunk.logicalDevice = finalizationChunk.logicalDevice;
    inputChunk.physicalDevice = finalizationChunk.physicalDevice;
    inputChunk.size = sizeof(float) * vertexLump.size();
    re = sizeof(float) * vertexLump.size();
    inputChunk.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    inputChunk.memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    Buffer stagingBuffer;

    vkUtil::createBuffer(inputChunk, stagingBuffer);

    inputChunk.memoryAllocatet = VkMemoryAllocateFlagBits::VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    //fill it with vertex data
    void *memoryLocation = nullptr;
    vkMapMemory(logicalDevice, stagingBuffer.bufferMemory, 0, inputChunk.size, 0, &memoryLocation);
    memcpy(memoryLocation, vertexLump.data(), inputChunk.size);
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
    inputChunk.size = sizeof(uint32_t) * indexLump.size();
    inputChunk.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    inputChunk.memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    vkUtil::createBuffer(inputChunk, stagingBuffer);


    vkMapMemory(logicalDevice, stagingBuffer.bufferMemory, 0, inputChunk.size, 0, &memoryLocation);
    memcpy(memoryLocation, indexLump.data(), inputChunk.size);
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


    // Staging buffer for indices
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
    accelerationStructureGeometry.geometry.triangles.maxVertex = VertexCount;
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
                                                               finalizationChunk.logicalDevice, bottomLevelAS,
                                                               accelerationStructureBuildSizesInfo);

    VkAccelerationStructureCreateInfoKHR accelerationStructureCreateInfo{};
    accelerationStructureCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
    accelerationStructureCreateInfo.buffer = bottomLevelAS.buffer;
    accelerationStructureCreateInfo.size = accelerationStructureBuildSizesInfo.accelerationStructureSize;
    accelerationStructureCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    vkCreateAccelerationStructureKHR(logicalDevice, &accelerationStructureCreateInfo, nullptr, &bottomLevelAS.handle);

    // Create a small scratch buffer used during build of the bottom level acceleration structure
    vkAccelerationStructure::RayTracingScratchBuffer scratchBuffer = vkAccelerationStructure::createScratchBuffer(
        finalizationChunk.physicalDevice, finalizationChunk.logicalDevice,
        accelerationStructureBuildSizesInfo.buildScratchSize);

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
    accelerationDeviceAddressInfo.accelerationStructure = bottomLevelAS.handle;
    bottomLevelAS.deviceAddress = vkGetAccelerationStructureDeviceAddressKHR(
        logicalDevice, &accelerationDeviceAddressInfo);

    deleteScratchBuffer(finalizationChunk.logicalDevice, scratchBuffer);
    create_top_acceleration_structure(finalizationChunk.physicalDevice, finalizationChunk.commandBuffer,
                                      finalizationChunk.queue, commandPool);
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

    const uint32_t numInstances = 2;  // Example: 3 instances referencing different BLAS
    VkAccelerationStructureInstanceKHR instances[numInstances] = {};
    
    // Populate each instance with a reference to a different BLAS
    for (uint32_t i = 0; i < numInstances; ++i) {
        instances[i].transform = transformMatrix;  // Assuming a shared transform matrix
        instances[i].instanceCustomIndex = i;
        instances[i].mask = 0xFF;
        instances[i].instanceShaderBindingTableRecordOffset = 0;
        instances[i].flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;

        // Assign the device address of the corresponding BLAS
        instances[i].accelerationStructureReference = bottomLevelAS.deviceAddress;  // Assuming an array of BLAS
        transform(glm::vec3(2,0,0));
    }

    // Buffer for instance data
    Buffer instanceBuffer;
    BufferInputChunk inputChunk{};
    inputChunk.logicalDevice = logicalDevice;
    inputChunk.physicalDevice = physicalDevice;
    inputChunk.size = sizeof(VkAccelerationStructureInstanceKHR) * numInstances;  // Size for multiple instances
    inputChunk.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    inputChunk.memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    Buffer stagingBuffer;

    vkUtil::createBuffer(inputChunk, stagingBuffer);

    inputChunk.memoryAllocatet = VkMemoryAllocateFlagBits::VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    // Fill it with vertex data (instance data)
    void *memoryLocation = nullptr;
    vkMapMemory(logicalDevice, stagingBuffer.bufferMemory, 0, inputChunk.size, 0, &memoryLocation);
    memcpy(memoryLocation, instances, inputChunk.size);  // Copy the instances array
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

    uint32_t primitive_count = numInstances;  // Number of instances

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
    accelerationStructureBuildRangeInfo.primitiveCount = numInstances;  // Adjusted for multiple instances
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

