#pragma once
#include "config.h"
#include <vector>
#include <unordered_map>
#include "View/vkAccelerationStructures/AccelerationStructure.h"

namespace vkAccelerationStructure {
	struct  FinalizationChunk {
		VkDevice logicalDevice;
		VkPhysicalDevice physicalDevice;
		VkQueue queue;
		VkCommandBuffer commandBuffer;

	};

	class VertexMenagerie
	{
	private:
		int indexOffset;

		VkDevice logicalDevice;

	public:
		VertexMenagerie();
		~VertexMenagerie();
		void consume(uint64_t meshType, std::vector<float> data, std::vector<uint32_t> indices);
		std::vector<float> vertexLump;
		std::vector<uint32_t> indexLump;

		void finalize(FinalizationChunk finalizationChunk, VkCommandPool commandPool,uint32_t& re);
		void create_top_acceleration_structure(VkPhysicalDevice physicalDevice,VkCommandBuffer commandBuffer, VkQueue queue, VkCommandPool commandPool);
		Buffer vertexBuffer, indexBuffer, transformBuffer;
		std::unordered_map<uint64_t, int> firstIndices;
		std::unordered_map<uint64_t, int> indexCounts;
		vkAccelerationStructure::AccelerationStructure  bottomLevelAS;
		vkAccelerationStructure::AccelerationStructure  topLevelAS;

	};
}