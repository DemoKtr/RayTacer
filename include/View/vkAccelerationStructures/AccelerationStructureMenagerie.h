#pragma once
#include "config.h"
#include <vector>
#include <unordered_map>

#include "glm/fwd.hpp"
#include "View/vkAccelerationStructures/AccelerationStructure.h"

namespace vkMesh {
	class ObjMesh;
}

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
		VkDevice logicalDevice;

	public:
		VertexMenagerie();
		~VertexMenagerie();

		void create_blas(vkAccelerationStructure::FinalizationChunk finalizationChunk,vkMesh::ObjMesh mesh,VkTransformMatrixKHR transformMatrix);
		
		void finalize(FinalizationChunk finalizationChunk, VkCommandPool commandPool,uint32_t& re);
		void create_top_acceleration_structure(VkPhysicalDevice physicalDevice,VkCommandBuffer commandBuffer, VkQueue queue, VkCommandPool commandPool);
		
		std::vector<vkAccelerationStructure::AccelerationStructure>  bottomLevelASes;
		std::vector<VkTransformMatrixKHR> transformMatrixes;
	
		uint32_t size = 0;
	
		vkAccelerationStructure::AccelerationStructure  topLevelAS;
		std::vector<uint32_t> extraBLASoffsets;
		std::vector<glm::vec4> inputArray;
		size_t totalExtraBLASBufferSize = 0; 
	};
}
