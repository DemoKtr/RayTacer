#include "View/vkUtil/Pipelines/rayTracingPipelineBuilder.h"
#include <View/vkUtil/Pipelines/shaders.h>
#include <View/vkUtil/memory.h>




VkPipelineShaderStageCreateInfo vkInit::RayTracingPipelineBuilder::make_shader_info(const VkShaderModule& shaderModule, const VkShaderStageFlagBits& stage) {
	VkPipelineShaderStageCreateInfo shaderInfo = {};
	shaderInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderInfo.flags = VkPipelineShaderStageCreateFlags();
	shaderInfo.stage = stage;
	shaderInfo.module = shaderModule;
	shaderInfo.pName = "main";
	return shaderInfo;
}

void vkInit::RayTracingPipelineBuilder::create_shader_groups(VkPipeline pipeline, VkQueue queue, VkCommandBuffer commandBuffer, Buffer& raygenShaderBindingTable, Buffer& missShaderBindingTable, Buffer& hitShaderBindingTable) {
	
	VkPhysicalDeviceRayTracingPipelinePropertiesKHR rtProps{};
	rtProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;

	VkPhysicalDeviceProperties2 deviceProps2{};
	deviceProps2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
	deviceProps2.pNext = &rtProps;
	vkGetPhysicalDeviceProperties2(physicalDevice, &deviceProps2);

	// Obliczamy wyrównanie uchwytów
	uint32_t handleSize = rtProps.shaderGroupHandleSize;
	uint32_t handleAlignment = rtProps.shaderGroupBaseAlignment;
	uint32_t handleStride = (handleSize + handleAlignment - 1) & ~(handleAlignment - 1);
	uint32_t sbtSize = handleStride * static_cast<uint32_t>(shaderGroups.size());

	// Rezerwujemy pamiêæ na uchwyty shaderów
	std::vector<uint8_t> shaderHandleStorage(sbtSize);

	// Pobieramy wskaŸnik do funkcji ray tracingu
	PFN_vkGetRayTracingShaderGroupHandlesKHR pfnGetRTShaderGroupHandles =
		(PFN_vkGetRayTracingShaderGroupHandlesKHR)vkGetDeviceProcAddr(device, "vkGetRayTracingShaderGroupHandlesKHR");
	if (!pfnGetRTShaderGroupHandles) {
		throw std::runtime_error("Nie uda³o siê pobraæ wskaŸnika vkGetRayTracingShaderGroupHandlesKHR");
	}
	VkResult result = pfnGetRTShaderGroupHandles(device, pipeline, 0, static_cast<uint32_t>(shaderGroups.size()), sbtSize, shaderHandleStorage.data());
	if (result != VK_SUCCESS) std::cout << "kurwa" << std::endl;
	const VkBufferUsageFlags bufferUsageFlags = VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR |
		VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
		VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	const VkMemoryPropertyFlags hostMemoryFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	// Przygotowujemy strukturê wejœciow¹ do tworzenia buforów
	BufferInputChunk inputChunk = {};
	inputChunk.logicalDevice = device;
	inputChunk.physicalDevice = physicalDevice;
	inputChunk.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	inputChunk.memoryProperties = hostMemoryFlags;
	// Kopiujemy tylko "u¿yteczn¹" czêœæ uchwytu
	inputChunk.size = handleSize;

	Buffer stagingBuffer;
	void* memoryLocation = nullptr;

	// ----- Raygen SBT -----
	// Tworzymy staging buffer
	vkUtil::createBuffer(inputChunk, stagingBuffer);

	// Mapujemy staging buffer i kopiujemy dane uchwytu raygen (offset 0)
	vkMapMemory(device, stagingBuffer.bufferMemory, 0, inputChunk.size, 0, &memoryLocation);
	memcpy(memoryLocation, shaderHandleStorage.data(), inputChunk.size);
	vkUnmapMemory(device, stagingBuffer.bufferMemory);

	// Tworzymy docelowy bufor raygen z pamiêci¹ DEVICE_LOCAL
	inputChunk.usage = bufferUsageFlags;
	inputChunk.memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	vkUtil::createBuffer(inputChunk, raygenShaderBindingTable);

	// Kopiujemy dane ze staging buffer do bufora docelowego
	vkUtil::copyBuffer(stagingBuffer, raygenShaderBindingTable, inputChunk.size, queue, commandBuffer);

	// Czyszczenie staging buffer
	vkDestroyBuffer(device, stagingBuffer.buffer, nullptr);
	vkFreeMemory(device, stagingBuffer.bufferMemory, nullptr);

	// ----- Miss SBT -----
	// Dla miss shader – offset o jeden uchwyt (handleStride)
	inputChunk.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	inputChunk.memoryProperties = hostMemoryFlags;
	vkUtil::createBuffer(inputChunk, stagingBuffer);

	vkMapMemory(device, stagingBuffer.bufferMemory, 0, inputChunk.size, 0, &memoryLocation);
	memcpy(memoryLocation, shaderHandleStorage.data() + handleStride, inputChunk.size);
	vkUnmapMemory(device, stagingBuffer.bufferMemory);

	inputChunk.usage = bufferUsageFlags;
	inputChunk.memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	vkUtil::createBuffer(inputChunk, missShaderBindingTable);

	vkUtil::copyBuffer(stagingBuffer, missShaderBindingTable, inputChunk.size, queue, commandBuffer);

	vkDestroyBuffer(device, stagingBuffer.buffer, nullptr);
	vkFreeMemory(device, stagingBuffer.bufferMemory, nullptr);

	// ----- Hit SBT -----
	// Dla hit shader – offset o dwa uchwyty (handleStride * 2)
	inputChunk.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	inputChunk.memoryProperties = hostMemoryFlags;
	vkUtil::createBuffer(inputChunk, stagingBuffer);

	vkMapMemory(device, stagingBuffer.bufferMemory, 0, inputChunk.size, 0, &memoryLocation);
	memcpy(memoryLocation, shaderHandleStorage.data() + handleStride * 2, inputChunk.size);
	vkUnmapMemory(device, stagingBuffer.bufferMemory);

	inputChunk.usage = bufferUsageFlags;
	inputChunk.memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	vkUtil::createBuffer(inputChunk, hitShaderBindingTable);

	vkUtil::copyBuffer(stagingBuffer, hitShaderBindingTable, inputChunk.size, queue, commandBuffer);

	vkDestroyBuffer(device, stagingBuffer.buffer, nullptr);
	vkFreeMemory(device, stagingBuffer.bufferMemory, nullptr);

}

uint32_t vkInit::alignedSize(uint32_t value, uint32_t alignment)
{
	return (value + alignment - 1) & ~(alignment - 1);
}

vkInit::RayTracingPipelineBuilder::RayTracingPipelineBuilder(VkPhysicalDevice physicalDevice, VkDevice device) {
	this->physicalDevice = physicalDevice;
	this->device = device;
}

vkInit::RayTracingPipelineBuilder::~RayTracingPipelineBuilder()
{
}

VkPipelineLayout vkInit::RayTracingPipelineBuilder::make_pipeline_layout(VkPipelineLayout& pipelineLayout) {
	/*
	typedef struct VkPipelineLayoutCreateInfo {
		VkStructureType                 sType;
		const void*                     pNext;
		VkPipelineLayoutCreateFlags     flags;
		uint32_t                        setLayoutCount;
		const VkDescriptorSetLayout*    pSetLayouts;
		uint32_t                        pushConstantRangeCount;
		const VkPushConstantRange*      pPushConstantRanges;
	} VkPipelineLayoutCreateInfo;
	*/

	VkPipelineLayoutCreateInfo layoutInfo;
	layoutInfo.flags = VkPipelineLayoutCreateFlags();
	layoutInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	layoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
	layoutInfo.pSetLayouts = descriptorSetLayouts.data();
	layoutInfo.pNext = nullptr;



	layoutInfo.pushConstantRangeCount = pushConstantCount;
	layoutInfo.pPushConstantRanges = &pushConstantInfo;




	VkResult result = vkCreatePipelineLayout(device, &layoutInfo, nullptr, &pipelineLayout);

	if (result != VK_SUCCESS) {
		std::cerr << "Failed to create pipeline layout! Error code: " << result << std::endl;
		return VK_NULL_HANDLE;
	}
}

void vkInit::RayTracingPipelineBuilder::specify_ray_gen_shader(const char* filename) {
	if (rayGenShader) {
		vkDestroyShaderModule(device, rayGenShader, nullptr);
		rayGenShader = nullptr;
	}

	std::cout << "Create RayGen shader module" << std::endl;
	vkUtil::createModule(filename, device, rayGenShader);
	rayGenShaderInfo = make_shader_info(rayGenShader, VkShaderStageFlagBits::VK_SHADER_STAGE_RAYGEN_BIT_KHR);
	
	shaderStages.push_back(rayGenShaderInfo);


	VkRayTracingShaderGroupCreateInfoKHR shaderGroup{};
	shaderGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
	shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
	shaderGroup.generalShader = static_cast<uint32_t>(shaderStages.size()) - 1;
	shaderGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
	shaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
	shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
	shaderGroups.push_back(shaderGroup);

}

void vkInit::RayTracingPipelineBuilder::specify_miss_shader(const char* filename) {
	if (missShader) {
		vkDestroyShaderModule(device, missShader, nullptr);
		missShader = nullptr;
	}

	std::cout << "Create Miss shader module" << std::endl;
	vkUtil::createModule(filename, device, missShader);
	missShaderInfo = make_shader_info(missShader, VkShaderStageFlagBits::VK_SHADER_STAGE_MISS_BIT_KHR);

	shaderStages.push_back(rayGenShaderInfo);

	VkRayTracingShaderGroupCreateInfoKHR shaderGroup{};
	shaderGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
	shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
	shaderGroup.generalShader = static_cast<uint32_t>(shaderStages.size()) - 1;
	shaderGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
	shaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
	shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
	shaderGroups.push_back(shaderGroup);
}

void vkInit::RayTracingPipelineBuilder::specify_closest_hit_shader(const char* filename) {
	if (closestHitShader) {
		vkDestroyShaderModule(device, closestHitShader, nullptr);
		closestHitShader = nullptr;
	}

	std::cout << "Create ClosestHit shader module" << std::endl;
	vkUtil::createModule(filename, device, missShader);
	missShaderInfo = make_shader_info(missShader, VkShaderStageFlagBits::VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);

	shaderStages.push_back(missShaderInfo);

	VkRayTracingShaderGroupCreateInfoKHR shaderGroup{};
	shaderGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
	shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
	shaderGroup.generalShader = VK_SHADER_UNUSED_KHR;
	shaderGroup.closestHitShader = static_cast<uint32_t>(shaderStages.size()) - 1;
	shaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
	shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
	shaderGroups.push_back(shaderGroup);
}

void vkInit::RayTracingPipelineBuilder::specify_all_hit_shader(const char* filename) {
	if (allHitShader) {
		vkDestroyShaderModule(device, allHitShader, nullptr);
		allHitShader = nullptr;
	}

	std::cout << "Create AllHit shader module" << std::endl;
	vkUtil::createModule(filename, device, allHitShader);
	allHitShaderInfo = make_shader_info(allHitShader, VkShaderStageFlagBits::VK_SHADER_STAGE_ANY_HIT_BIT_KHR);

	shaderStages.push_back(allHitShaderInfo);

	VkRayTracingShaderGroupCreateInfoKHR shaderGroup{};
	shaderGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
	shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
	shaderGroup.generalShader = VK_SHADER_UNUSED_KHR;
	shaderGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
	shaderGroup.anyHitShader = static_cast<uint32_t>(shaderStages.size()) - 1;
	shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
	shaderGroups.push_back(shaderGroup);

}



vkUtil::GraphicsPipelineOutBundle vkInit::RayTracingPipelineBuilder::build(VkQueue queue, VkCommandBuffer commandBuffer,Buffer& raygenShaderBindingTable, Buffer& missShaderBindingTable, Buffer& hitShaderBindingTable) {

	vkUtil::GraphicsPipelineOutBundle output;

	VkPipelineLayout pipelineLayout;
	make_pipeline_layout(pipelineLayout);

	VkRayTracingPipelineCreateInfoKHR rayTracingPipelineCI{};
	rayTracingPipelineCI.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
	rayTracingPipelineCI.stageCount = static_cast<uint32_t>(this->shaderStages.size());
	rayTracingPipelineCI.pStages = shaderStages.data();
	rayTracingPipelineCI.groupCount = static_cast<uint32_t>(shaderGroups.size());
	rayTracingPipelineCI.pGroups = shaderGroups.data();
	rayTracingPipelineCI.maxPipelineRayRecursionDepth = 1;
	rayTracingPipelineCI.layout = pipelineLayout;
	VkPipeline pipeline;

	PFN_vkCreateRayTracingPipelinesKHR vkCreateRayTracingPipelinesKHR;
	vkCreateRayTracingPipelinesKHR = (PFN_vkCreateRayTracingPipelinesKHR)(vkGetDeviceProcAddr(device, "vkCreateRayTracingPipelinesKHR"));

	vkCreateRayTracingPipelinesKHR(device, VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &rayTracingPipelineCI, nullptr, &pipeline);
	output.pipeline = pipeline;
	output.layout = pipelineLayout;
	
	create_shader_groups(pipeline, queue, commandBuffer,raygenShaderBindingTable, missShaderBindingTable, hitShaderBindingTable);
	return output;
}

void vkInit::RayTracingPipelineBuilder::add_descriptor_set_layout(VkDescriptorSetLayout descriptorSetLayout) {
	
	descriptorSetLayouts.push_back(descriptorSetLayout);
}

