#pragma once
#include "config.h"
#include <vector>
#include <unordered_map>

#include "glm/fwd.hpp"
#include "View/vkAccelerationStructures/AccelerationStructure.h"

namespace vkAccelerationStructure {
	enum class PrefabType {
		PLANE, 
		CUBE, 
		SPHERE
	};
	
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
		void consume(PrefabType prefabType, std::vector<float> data, std::vector<uint32_t> indices);
		//void consume(PrefabType prefabType);
		void transform(glm::vec3 vector);
		int VertexCount;
		VkTransformMatrixKHR transformMatrix;
		std::vector<float> vertexLump;
		std::vector<uint32_t> indexLump;
		uint32_t numTriangles = 3;

		void finalize(FinalizationChunk finalizationChunk, VkCommandPool commandPool,uint32_t& re);
		void create_top_acceleration_structure(VkPhysicalDevice physicalDevice,VkCommandBuffer commandBuffer, VkQueue queue, VkCommandPool commandPool);
		Buffer vertexBuffer, indexBuffer, transformBuffer;
		std::unordered_map<PrefabType, int> firstIndices;
		std::unordered_map<PrefabType, int> indexCounts;
		vkAccelerationStructure::AccelerationStructure  bottomLevelAS;
		vkAccelerationStructure::AccelerationStructure  topLevelAS;

	};
}