#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "View/graphicsEngine.h"
#include "View/vkInit/instance.h"
#include "View/vkInit/devices.h"
#include "View/vkUtil/queues.h"
#include <View/vkInit/swapchain.h>
#include <View/vkUtil/Pipelines/pipelineCache.h>
#include <View/vkUtil/Pipelines/computePipelineBuilder.h>
#include "View/vkUtil/Pipelines/rayTracingPipelineBuilder.h"
#include <View/vkInit/descriptors.h>
#include <View/vkInit/synchronizer.h>
#include <View/vkInit/commands.h>
#include <View/vkResources/resources.h>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

#include "View/vkMesh/obj_mesh.h"


GraphicsEngine::GraphicsEngine(GLFWwindow* window, int width, int height, bool debugMode) {
	this->width = width;
	this->height = height;
	this->mainWindow = window;
	this->debugMode = debugMode;
	if (debugMode) {
		std::cout << "Making a graphics engine\n";
	}



	make_instance();
	choice_device();
	vkResources::scenePipelines = new vkUtil::PipelineCache(device);
	create_swapchain();
	create_descriptor_set_layouts();
	create_frame_command_buffer();
	
	make_assets();

	create_pipeline();

	
	finalize_setup();
}

void GraphicsEngine::render() {
	

	vkWaitForFences(device, 1, &swapchainFrames[frameNumber].inFlight, VK_TRUE, UINT64_MAX);
	vkResetFences(device, 1, &swapchainFrames[frameNumber].inFlight);


		uint32_t imageIndex;
	
		VkResult result = vkAcquireNextImageKHR(
			device,
			swapchain,
			UINT64_MAX,
			swapchainFrames[frameNumber].imageAvailable, // przyk�ad: semafor dost�pno�ci obrazu
			nullptr,
			&imageIndex
		);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_ERROR_INCOMPATIBLE_DISPLAY_KHR) {
			std::cout << "Recreate" << std::endl;
			recreate_swapchain();
			return;
		}
		else if (result == VK_SUBOPTIMAL_KHR || result == VK_TIMEOUT) {
			std::cout << "timeout" << std::endl;
			return;
		}
		else if (result != VK_SUCCESS) {
			std::cout << "Failed to acquire swapchain image! Error code: " << result << std::endl;
			return;
		}


		

		prepare_frame(frameNumber);

		VkCommandBuffer MainCommandBuffer = swapchainFrames[frameNumber].mainCommandBuffer;
		//VkCommandBuffer computeCommandBuffer = swapchainFrames[frameNumber].computeCommandBuffer;

	
		vkResetCommandBuffer(MainCommandBuffer, VkCommandBufferResetFlagBits::VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
		//vkResetCommandBuffer(computeCommandBuffer, VkCommandBufferResetFlagBits::VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);




		record_draw_command(MainCommandBuffer,imageIndex);
		//record_raytracing_command(computeCommandBuffer,imageIndex);



		VkSubmitInfo submitInfo = { };

		VkSemaphore waitSemaphores[] = { swapchainFrames[frameNumber].imageAvailable };
		VkPipelineStageFlags  waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = 1;
		// Graphics czeka na zako�czenie compute:
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &MainCommandBuffer;

		
		VkSemaphore signalSemaphores[] = { swapchainFrames[frameNumber].renderFinished };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		result = vkQueueSubmit(graphicsQueue, 1, &submitInfo, swapchainFrames[frameNumber].inFlight);
		if (result != VK_SUCCESS) {
			std::cout << "Synchronization failed" << std::endl;
		}


		VkPresentInfoKHR presentInfo = { };
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &swapchainFrames[frameNumber].renderFinished;
		VkSwapchainKHR swapChains[] = { swapchain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.pNext = nullptr;

		result = vkQueuePresentKHR(presentQueue, &presentInfo);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
			std::cout << "Recreate" << std::endl;
			recreate_swapchain();
			return;
		}


		frameNumber = (frameNumber + 1) % maxFramesInFlight;
	
}

GraphicsEngine::~GraphicsEngine() {
	delete vkResources::scenePipelines;
	delete accelerationStructure;
	vkDestroyCommandPool(device, CommandPool, nullptr);
	vkDestroyCommandPool(device, computeCommandPool, nullptr);
	vkDestroyCommandPool(device, transferCommandPool, nullptr);

	cleanup_swapchain();

	vkDestroyDevice(device, nullptr);
	vkDestroySurfaceKHR(instance, surface, nullptr);
	if (debugMode) {
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr) {
			func(instance, debugMessenger, nullptr);
		}
	}
	vkDestroyInstance(instance, nullptr);
	glfwTerminate();
}

void GraphicsEngine::make_instance() {
	vkInit::make_instance(this->instance, debugMode, appName);
	VkSurfaceKHR c_style_surface;
	if (glfwCreateWindowSurface(instance, mainWindow, nullptr, &c_style_surface) != VK_SUCCESS) {
		if (debugMode) {
			std::cout << "Failed to abstract glfw surface for Vulkan\n";
		}
	}
	else if (debugMode) {
		std::cout << "Successfully abstracted glfw surface for Vulkan\n";
	}
	surface = c_style_surface;
}
void GraphicsEngine::choice_device() {
	vkInit::choose_physical_device(this->instance,physicalDevice ,debugMode);
	
	vkInit::create_logical_device(physicalDevice,device ,surface, debugMode);
	
	VkQueue queues[4];
	vkInit::get_Queues(physicalDevice, device, queues,surface, debugMode);

	this->graphicsQueue = queues[0];

	this->presentQueue = queues[1];

	this->computeQueue = queues[2];
	this->transferQueue = queues[3];


	//this->create_swapchain();

	frameNumber = 0;
}
void GraphicsEngine::create_swapchain() {

	vkInit::SwapChainBundle bundle = vkInit::create_swapchain(physicalDevice, device, surface, width,height, debugMode);
	this->swapchain = bundle.swapchain;
	this->swapchainFrames = bundle.frames;
	this->swapchainFormat = bundle.format;
	this->swapchainExtent = bundle.extent;

	vkInit::query_swapchain_support(physicalDevice, surface, debugMode);
	maxFramesInFlight = static_cast<int>(swapchainFrames.size());

	for (vkUtil::SwapChainFrame& frame : swapchainFrames) {
		frame.logicalDevice = device;
		frame.physicalDevice = physicalDevice;


	}
	frameNumber = 0;
	std::cout << maxFramesInFlight << std::endl;
}

void GraphicsEngine::recreate_swapchain() {
	this->width = 0;
	this->height = 0;
	while (this->width == 0 || this->height == 0) {
		glfwGetFramebufferSize(mainWindow, &this->width, &this->height);
		glfwWaitEvents();
	}

	
	vkDeviceWaitIdle(device);

	cleanup_swapchain();
	create_swapchain();
}

void GraphicsEngine::cleanup_swapchain() {
	uint32_t i = 0;
	for (vkUtil::SwapChainFrame& frame : swapchainFrames) {
		frame.destroy();
		i++;
	}
	vkDestroySwapchainKHR(device, swapchain, nullptr);
}

void GraphicsEngine::create_pipeline() {
	vkUtil::PipelineCacheChunk pipeline;
	

	vkInit::PipelineBuilder pipelineBuilder(device);
	pipelineBuilder.set_overwrite_mode(true);
	pipelineBuilder.specify_vertex_shader("resources/shaders/vert.spv");
	pipelineBuilder.specify_fragment_shader("resources/shaders/frag.spv");
	pipelineBuilder.specify_swapchain_extent(swapchainExtent);
	pipelineBuilder.clear_depth_attachment();
	pipelineBuilder.set_color_blending(false);
	pipelineBuilder.add_descriptor_set_layout(finalImageDescriptorSetLayout);
	pipelineBuilder.use_depth_test(false);
	pipelineBuilder.dynamicRendering = true;
	vkUtil::GraphicsPipelineOutBundle output = pipelineBuilder.build(swapchainFormat);
	pipeline.pipelineLayout = output.layout;
	pipeline.pipeline = output.pipeline;
	vkResources::scenePipelines->addPipeline("Finall Image", pipeline);
	//pipelineBuilder.reset();
	
	vkInit::ComputePipelineBuilder computePipelineBuilder(device);
	computePipelineBuilder.add_descriptor_set_layout(rayCastDescriptorSetLayout);
	//computePipelineBuilder.set_push_constant();
	computePipelineBuilder.specify_compute_shader("resources/shaders/compute.spv");
	output = computePipelineBuilder.build(true);
	pipeline.pipelineLayout = output.layout;
	pipeline.pipeline = output.pipeline;

	vkResources::scenePipelines->addPipeline("RayCast Pipeline", pipeline);
	//computePipelineBuilder.reset();
	
	vkInit::RayTracingPipelineBuilder rayBuilder(physicalDevice,device);
	rayBuilder.add_descriptor_set_layout(rayGenDescriptorSetLayout);
	rayBuilder.add_descriptor_set_layout(textureDescriptorSetLayout);
	rayBuilder.specify_ray_gen_shader("resources/shaders/raygen.spv",1);
	rayBuilder.specify_miss_shader("resources/shaders/miss.spv");
	rayBuilder.specify_miss_shader("resources/shaders/miss_shadow.spv");
	rayBuilder.specify_closest_hit_shader("resources/shaders/closesthit.spv");
	//rayBuilder.specify_all_hit_shader("resources/shaders/anyhit_shadow.spv");
	output = rayBuilder.build(graphicsQueue,maincommandBuffer, raygenShaderBindingTable, missShaderBindingTable, hitShaderBindingTable);
	pipeline.pipelineLayout = output.layout;
	pipeline.pipeline = output.pipeline;
	
	vkResources::scenePipelines->addPipeline("Ray", pipeline);

	vkInit::RayTracingPipelineBuilder rayPBRBuilder(physicalDevice, device);
	rayPBRBuilder.add_descriptor_set_layout(rayGenDescriptorSetLayout);
	rayPBRBuilder.add_descriptor_set_layout(textureDescriptorSetLayout);
	rayPBRBuilder.specify_ray_gen_shader("resources/shaders/rg.spv",4);
	rayPBRBuilder.specify_miss_shader("resources/shaders/m.spv");
	rayPBRBuilder.specify_miss_shader("resources/shaders/miss_shadow.spv");
	rayPBRBuilder.specify_closest_hit_shader("resources/shaders/ch.spv");
	//rayBuilder.specify_all_hit_shader("resources/shaders/anyhit_shadow.spv");
	output = rayPBRBuilder.build(graphicsQueue, maincommandBuffer, raygenShaderBindingTablePBR, missShaderBindingTablePBR, hitShaderBindingTablePBR);
	pipeline.pipelineLayout = output.layout;
	pipeline.pipeline = output.pipeline;

	vkResources::scenePipelines->addPipeline("RayPBR", pipeline);
}

void GraphicsEngine::finalize_setup() {
	create_frame_resources();
	
}

void GraphicsEngine::make_assets() {


	vkInit::descriptorSetLayoutData bindings;
	bindings.count = 1;
	bindings.types.push_back(VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	vkInit::make_descriptor_pool(device,textureDescriptorPool ,static_cast<uint32_t>(2), bindings);

	std::vector<std::string> texturesNames;
	
	texturesNames.push_back("resources/textures/white1.png");
	texturesNames.push_back("resources/textures/black.png");
	
	// color normal arm dissplacement
	texturesNames.push_back("resources/textures/groundDiff.png");
	texturesNames.push_back("resources/textures/groundNormal.png");
	texturesNames.push_back("resources/textures/groundARM.png");
	texturesNames.push_back("resources/textures/groundDiss.png");

	texturesNames.push_back("resources/textures/floorDiff.png");
	texturesNames.push_back("resources/textures/floorNormal.png");
	texturesNames.push_back("resources/textures/floorARM.png");
	texturesNames.push_back("resources/textures/floorDiss.png");

	texturesNames.push_back("resources/textures/metalDiff.png");
	texturesNames.push_back("resources/textures/metalNormal.png");
	texturesNames.push_back("resources/textures/metalARM.png");
	texturesNames.push_back("resources/textures/metalDiss.png");

	texturesNames.push_back("resources/textures/rockDiff.png");
	texturesNames.push_back("resources/textures/rockNormal.png");
	texturesNames.push_back("resources/textures/rockARM.png");
	texturesNames.push_back("resources/textures/rockDiss.png");

	texturesNames.push_back("resources/textures/stoneDiff.png");
	texturesNames.push_back("resources/textures/stoneNormal.png");
	texturesNames.push_back("resources/textures/stoneARM.png");
	texturesNames.push_back("resources/textures/stoneDiss.png");

	texturesNames.push_back("resources/textures/woodDiff.png");
	texturesNames.push_back("resources/textures/woodNormal.png");
	texturesNames.push_back("resources/textures/woodARM.png");
	texturesNames.push_back("resources/textures/woodDiss.png");


	vkImage::TextureInputChunk inputTexture;
	inputTexture.logicalDevice = device;
	inputTexture.physicalDevice = physicalDevice;
	inputTexture.texturesNames = texturesNames;
	inputTexture.filenames = nullptr;
	inputTexture.queue = graphicsQueue;
	inputTexture.commandBuffer = maincommandBuffer;
	inputTexture.layout = textureDescriptorSetLayout;
	inputTexture.descriptorPool = textureDescriptorPool;

	textures = new vkImage::Texture(inputTexture);



	accelerationStructure = new vkAccelerationStructure::VertexMenagerie();
	vkAccelerationStructure::FinalizationChunk input;
	input.logicalDevice = device;
	input.physicalDevice = physicalDevice;
	input.queue = graphicsQueue;
	input.commandBuffer = maincommandBuffer;
	
	glm::mat4 glmMatrix(1); // Assume this is your populated glm::mat4 matrix

	VkTransformMatrixKHR vkMatrix;
	glmMatrix = translate(glmMatrix, glm::vec3(0,0,0));

	
	for(int i = 0; i < 3; i++)
	{
		for(int j = 0; j < 4; j++)
		{
			vkMatrix.matrix[i][j] = glmMatrix[j][i]; // Note the swapped indices due to column-major vs row-major discrepancy
		}
	}
	//glm::vec4 in = glm::vec4(0,0,0,0);
	glm::vec4 in = glm::vec4(18, 19, 20, 21);
	accelerationStructure->create_blas(input,vkMesh::ObjMesh("resources/models/c1.obj", "resources/models/sphere1.mtl",glm::mat4(1.0f)),vkMatrix,in);
	in = glm::vec4(1, 0, 0, 0);
	accelerationStructure->create_blas(input,vkMesh::ObjMesh("resources/models/c2.obj", "resources/models/sphere1.mtl",glm::mat4(1.0f)),vkMatrix,in);
	in = glm::vec4(2, 3, 4, 5);
	accelerationStructure->create_blas(input,vkMesh::ObjMesh("resources/models/Plane.obj", "resources/models/sphere1.mtl",glm::mat4(1.0f)),vkMatrix, in);
	in = glm::vec4(6, 7, 8, 9);
	accelerationStructure->create_blas(input,vkMesh::ObjMesh("resources/models/Plane1.obj", "resources/models/sphere1.mtl",glm::mat4(1.0f)),vkMatrix, in);
	in = glm::vec4(10, 11, 12, 13);
	accelerationStructure->create_blas(input,vkMesh::ObjMesh("resources/models/Plane2.obj", "resources/models/sphere1.mtl",glm::mat4(1.0f)),vkMatrix, in);
	in = glm::vec4(14, 15,16, 17);
	accelerationStructure->create_blas(input,vkMesh::ObjMesh("resources/models/Plane3.obj", "resources/models/sphere1.mtl",glm::mat4(1.0f)),vkMatrix, in);
	in = glm::vec4(18, 19, 20, 21);
	accelerationStructure->create_blas(input,vkMesh::ObjMesh("resources/models/Plane4.obj", "resources/models/sphere1.mtl",glm::mat4(1.0f)),vkMatrix, in);
	in = glm::vec4(22, 23, 24, 25);
	accelerationStructure->create_blas(input,vkMesh::ObjMesh("resources/models/Plane5.obj", "resources/models/sphere1.mtl",glm::mat4(1.0f)),vkMatrix, in);

	
	//accelerationStructure->create_blas(input,vkMesh::ObjMesh("resources/models/sphere1.obj", "resources/models/sphere1.mtl",glm::mat4(1.0f)),vkMatrix);


	glmMatrix = translate(glmMatrix, glm::vec3(-2,0,0));
	
	for(int i = 0; i < 3; i++)
	{
		for(int j = 0; j < 4; j++)
		{
			vkMatrix.matrix[i][j] = glmMatrix[j][i]; // Note the swapped indices due to column-major vs row-major discrepancy
		}
	}
	
	//accelerationStructure->create_blas(input,vkMesh::ObjMesh("resources/models/sphere.obj", "resources/models/sphere.mtl",glm::mat4(1.0f)),vkMatrix);

	accelerationStructure->finalize(input, CommandPool, bufferSize);
}


void GraphicsEngine::create_frame_command_buffer() {
	
	vkInit::make_command_pool(physicalDevice, device,CommandPool ,surface, debugMode);

	vkInit::make_compute_command_pool(physicalDevice, device, computeCommandPool,surface, debugMode);
	vkInit::make_transfer_command_pool(physicalDevice, device, transferCommandPool,surface, debugMode);

	vkInit::commandBufferInputChunk commandBufferInput = { device,CommandPool, swapchainFrames};

	vkInit::make_command_buffer(commandBufferInput,maincommandBuffer ,debugMode);
	vkInit::make_frame_command_buffers(commandBufferInput, debugMode);

	commandBufferInput.commandPool = computeCommandPool;

	vkInit::make_frame_compute_command_buffers(commandBufferInput, debugMode);

	vkInit::make_command_buffer(commandBufferInput,computeCommandBuffer ,debugMode);
	commandBufferInput.commandPool = transferCommandPool;

	vkInit::make_command_buffer(commandBufferInput,transferCommandBuffer ,debugMode);
	
}

void GraphicsEngine::create_frame_resources() {
	
	vkInit::descriptorSetLayoutData bindings;
	bindings.count = 1;
	bindings.types.push_back(VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

	vkInit::make_descriptor_pool(device, finalImageDescriptorPool,static_cast<uint32_t>(swapchainFrames.size()), bindings);

	bindings.count = 3;
	bindings.types[0] = VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	bindings.types.push_back(VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	bindings.types.push_back(VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	vkInit::make_descriptor_pool(device, rayCastDescriptorPool, static_cast<uint32_t>(swapchainFrames.size()), bindings);

	bindings.count = 7;
	bindings.types[0] = VkDescriptorType::VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
	bindings.types.push_back(VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
	bindings.types.push_back(VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	bindings.types.push_back(VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER); //material
	bindings.types.push_back(VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER); //lights
	bindings.types.push_back(VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER); //texcord an normals
	bindings.types.push_back(VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER); //offsets

	vkInit::make_descriptor_pool(device, rayGenDescriptorPool, static_cast<uint32_t>(swapchainFrames.size()), bindings);
	for (vkUtil::SwapChainFrame& frame : swapchainFrames) //referencja 
	{
		vkInit::make_semaphore(device, frame.imageAvailable,debugMode);
		vkInit::make_semaphore(device, frame.renderFinished,debugMode);
		vkInit::make_semaphore(device, frame.computeFinished,debugMode);
		vkInit::make_fence(device, frame.inFlight,debugMode);
		frame.make_descriptors_resources(accelerationStructure);
		vkInit::allocate_descriptor_set(device, frame.postProcessDescriptorSet,finalImageDescriptorPool, finalImageDescriptorSetLayout);
		vkInit::allocate_descriptor_set(device, frame.RayCastDescriptorSet,rayCastDescriptorPool, rayCastDescriptorSetLayout);
		vkInit::allocate_descriptor_set(device, frame.RayGenDescriptorSet,rayGenDescriptorPool, rayGenDescriptorSetLayout);
		


	}
	
}

void GraphicsEngine::create_descriptor_set_layouts() {
	
	vkInit::descriptorSetLayoutData bindings;
	bindings.count = 1;
	bindings.indices.push_back(0);
	bindings.types.push_back(VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	bindings.counts.push_back(1);
	bindings.stages.push_back(VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT);



	vkInit::make_descriptor_set_layout(device, bindings, finalImageDescriptorSetLayout);
	bindings.stages[0] = VkShaderStageFlagBits::VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
	vkInit::make_descriptor_set_layout(device, bindings, textureDescriptorSetLayout);
	bindings.types[0] = VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	bindings.stages[0] = (VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT);
	vkInit::make_descriptor_set_layout(device, bindings, rayCastDescriptorSetLayout);


	bindings.count = 7;
	
	bindings.types[0] = VkDescriptorType::VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
	bindings.stages[0] = (VkShaderStageFlagBits::VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);

	bindings.indices.push_back(1);
	bindings.types.push_back(VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
	bindings.counts.push_back(1);
	bindings.stages.push_back(VkShaderStageFlagBits::VK_SHADER_STAGE_RAYGEN_BIT_KHR);

	bindings.indices.push_back(2);
	bindings.types.push_back(VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	bindings.counts.push_back(1);
	bindings.stages.push_back(VkShaderStageFlagBits::VK_SHADER_STAGE_RAYGEN_BIT_KHR);

	bindings.indices.push_back(3);
	bindings.types.push_back(VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	bindings.counts.push_back(1);
	bindings.stages.push_back(VkShaderStageFlagBits::VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);

	bindings.indices.push_back(4);
	bindings.types.push_back(VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	bindings.counts.push_back(1);
	bindings.stages.push_back(VkShaderStageFlagBits::VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);

	bindings.indices.push_back(5);
	bindings.types.push_back(VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	bindings.counts.push_back(1);
	bindings.stages.push_back(VkShaderStageFlagBits::VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);
	
	bindings.indices.push_back(6);
	bindings.types.push_back(VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	bindings.counts.push_back(1);
	bindings.stages.push_back(VkShaderStageFlagBits::VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);
	
	vkInit::make_descriptor_set_layout(device, bindings, rayGenDescriptorSetLayout);
	
}

void GraphicsEngine::record_draw_command(VkCommandBuffer commandBuffer, uint32_t imageIndex) {

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0; // lub VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, je�li dotyczy
	beginInfo.pInheritanceInfo = NULL;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);


	VkImageMemoryBarrier barrier = { };
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL; // lub inny oczekiwany layout
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = swapchainFrames[imageIndex].postProcessImage; // Twoja zmienna obrazu
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.srcAccessMask = 0; // poniewa� UNDEFINED nie wymaga synchronizacji
	barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT; // zale�y od dalszego u�ycia

	vkCmdPipelineBarrier(
		commandBuffer,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, // lub inny odpowiedni etap
		0, 0, NULL, 0, NULL, 1, &barrier
	);
	vkUtil::PipelineCacheChunk pipelineInfo;
	if(!PBR)
	pipelineInfo = vkResources::scenePipelines->getPipeline("Ray");
	else
	pipelineInfo = vkResources::scenePipelines->getPipeline("RayPBR");

	VkPhysicalDeviceRayTracingPipelinePropertiesKHR rtProps{};
	rtProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
	VkPhysicalDeviceProperties2 deviceProps2{};
	deviceProps2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
	deviceProps2.pNext = &rtProps;
	vkGetPhysicalDeviceProperties2(physicalDevice, &deviceProps2);

	// Dispatch zadania obliczeniowego
	const uint32_t handleSizeAligned = vkInit::alignedSize(rtProps.shaderGroupHandleSize,
		rtProps.shaderGroupHandleAlignment);




	VkStridedDeviceAddressRegionKHR raygenShaderSbtEntry{};
	//raygenShaderSbtEntry.deviceAddress = vkUtil::getBufferDeviceAddress(device, raygenShaderBindingTable.buffer);
	raygenShaderSbtEntry.stride = handleSizeAligned;
	raygenShaderSbtEntry.size = handleSizeAligned;

	VkStridedDeviceAddressRegionKHR missShaderSbtEntry{};
	//missShaderSbtEntry.deviceAddress = vkUtil::getBufferDeviceAddress(device, missShaderBindingTable.buffer);
	missShaderSbtEntry.stride = handleSizeAligned;
	missShaderSbtEntry.size = handleSizeAligned;

	VkStridedDeviceAddressRegionKHR hitShaderSbtEntry{};
	//hitShaderSbtEntry.deviceAddress = vkUtil::getBufferDeviceAddress(device, hitShaderBindingTable.buffer);
	hitShaderSbtEntry.stride = handleSizeAligned;
	hitShaderSbtEntry.size = handleSizeAligned;

	if (PBR) {
		hitShaderSbtEntry.deviceAddress = vkUtil::getBufferDeviceAddress(device, hitShaderBindingTablePBR.buffer);
		missShaderSbtEntry.deviceAddress = vkUtil::getBufferDeviceAddress(device, missShaderBindingTablePBR.buffer);
		raygenShaderSbtEntry.deviceAddress = vkUtil::getBufferDeviceAddress(device, raygenShaderBindingTablePBR.buffer);
	}
	else {
		hitShaderSbtEntry.deviceAddress = vkUtil::getBufferDeviceAddress(device, hitShaderBindingTable.buffer);
		missShaderSbtEntry.deviceAddress = vkUtil::getBufferDeviceAddress(device, missShaderBindingTable.buffer);
		raygenShaderSbtEntry.deviceAddress = vkUtil::getBufferDeviceAddress(device, raygenShaderBindingTable.buffer);
	}

	VkStridedDeviceAddressRegionKHR callableShaderSbtEntry{};
	

	/*
		Dispatch the ray tracing commands
	*/
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipelineInfo.pipeline);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipelineInfo.pipelineLayout, 0, 1,
		&swapchainFrames[imageIndex].RayGenDescriptorSet, 0, 0);
	textures->useTexture(commandBuffer, pipelineInfo.pipelineLayout);
	PFN_vkCmdTraceRaysKHR vkCmdTraceRaysKHR =
		(PFN_vkCmdTraceRaysKHR)vkGetDeviceProcAddr(device, "vkCmdTraceRaysKHR");
	vkCmdTraceRaysKHR(
		commandBuffer,
		&raygenShaderSbtEntry,
		&missShaderSbtEntry,
		&hitShaderSbtEntry,
		&callableShaderSbtEntry,
		swapchainExtent.width,
		swapchainExtent.height,
		1);

	/*
		Copy ray tracing output to swap chain image
	*/

	barrier = {  };
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL; // aktualny layout
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // oczekiwany layout
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = swapchainFrames[imageIndex].postProcessImage;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	// Ustal odpowiednio maski dost�pu (np. je�li obraz by� zapisywany, ustaw srcAccessMask na VK_ACCESS_MEMORY_WRITE_BIT)
	barrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(
		commandBuffer,
		VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, // lub odpowiedni etap, np. VK_PIPELINE_STAGE_TRANSFER_BIT
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, // lub etap, w kt�rym obraz ma by� u�yty
		0,
		0, NULL,
		0, NULL,
		1, &barrier
	);

	// Ustal kolor czyszczenia jako czarny (0, 0, 0, 1)
	VkClearColorValue clearColor = { { 0.0f, 0.0f, 0.0f, 1.0f } };
	pipelineInfo = vkResources::scenePipelines->getPipeline("Finall Image");
	VkRenderingAttachmentInfoKHR colorAttachment = {};
	colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
	colorAttachment.pNext = nullptr;
	colorAttachment.imageView = swapchainFrames[imageIndex].mainImageView; // Upewnij si�, �e imageView zosta� poprawnie utworzony
	colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.clearValue.color = clearColor;

	VkRenderingInfoKHR renderingInfo = {};
	renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
	renderingInfo.pNext = nullptr;
	renderingInfo.renderArea.offset = { 0, 0 };
	renderingInfo.renderArea.extent = swapchainExtent; 
	renderingInfo.layerCount = 1;
	renderingInfo.colorAttachmentCount = 1;
	renderingInfo.pColorAttachments = &colorAttachment;
	renderingInfo.pDepthAttachment = nullptr;
	PFN_vkCmdBeginRenderingKHR vkCmdBeginRenderingKHR =
		(PFN_vkCmdBeginRenderingKHR)vkGetDeviceProcAddr(device, "vkCmdBeginRenderingKHR");
	PFN_vkCmdEndRenderingKHR vkCmdEndRenderingKHR =
		(PFN_vkCmdEndRenderingKHR)vkGetDeviceProcAddr(device, "vkCmdEndRenderingKHR");
	
	vkCmdBeginRenderingKHR(commandBuffer, &renderingInfo);

	
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineInfo.pipeline);

	
	
	vkCmdBindDescriptorSets(
		commandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		pipelineInfo.pipelineLayout,
		0,                  // firstSet
		1,                  // descriptorSetCount
		&swapchainFrames[imageIndex].postProcessDescriptorSet,              // wska�nik do descriptor setu
		0,                  // dynamicOffsetCount
		nullptr             // pDynamicOffsets
	);


	vkCmdDraw(commandBuffer, 3, 1, 0, 0);

	// Zako�czenie renderowania dynamicznego
	vkCmdEndRenderingKHR(commandBuffer);



	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = swapchainFrames[imageIndex].mainImage;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	// Upewnij si�, �e wcze�niejsze operacje zapisu w attachment s� zako�czone
	barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	vkCmdPipelineBarrier(
		commandBuffer,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, // srcStageMask pozostaje, je�li to odpowiedni etap dla poprzednich operacji
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,   // zmieniony dstStageMask
		0,              // flagi bariery
		0, NULL,        // bariery pami�ci
		0, NULL,        // bariery bufor�w
		1, &barrier     // jedna bariera obrazu
	);
	vkEndCommandBuffer(commandBuffer);

	

}

void GraphicsEngine::record_compute_command(VkCommandBuffer commandBuffer , uint32_t imageIndex) {

	// Przygotowanie beginInfo dla command buffera

	

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.pNext = nullptr;
	beginInfo.flags = 0; // lub odpowiednie flagi, je�li potrzebujesz
	beginInfo.pInheritanceInfo = nullptr; // dla command buffer�w g��wnych

	VkResult result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
	if (result != VK_SUCCESS) {
		if (debugMode) {
			std::cout << "Failed to begin recording compute command buffer! Error code: " << result << std::endl;
		}
		// Mo�esz tutaj zako�czy� funkcj� lub podj�� inne dzia�anie
	}

	VkImageMemoryBarrier barrier = { };
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL; // lub inny oczekiwany layout
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = swapchainFrames[imageIndex].postProcessImage; // Twoja zmienna obrazu
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.srcAccessMask = 0; // poniewa� UNDEFINED nie wymaga synchronizacji
	barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT; // zale�y od dalszego u�ycia

	vkCmdPipelineBarrier(
		commandBuffer,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, // lub inny odpowiedni etap
		0, 0, NULL, 0, NULL, 1, &barrier
	);
	vkUtil::PipelineCacheChunk pipelineInfo = vkResources::scenePipelines->getPipeline("RayCast Pipeline");
	// Dispatch zadania obliczeniowego
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineInfo.pipeline);
	vkCmdBindDescriptorSets(
		commandBuffer,
		VK_PIPELINE_BIND_POINT_COMPUTE,
		pipelineInfo.pipelineLayout,
		0,                                   // firstSet
		1,                                   // descriptorSetCount
		&swapchainFrames[imageIndex].RayCastDescriptorSet,
		0,                                   // dynamicOffsetCount
		nullptr                             // pDynamicOffsets
	);
	vkCmdDispatch(commandBuffer,
		1920/16,
		(1080+16)/16,
		1
	);



	

	// Zako�czenie rejestrowania command buffera
	result = vkEndCommandBuffer(commandBuffer);
	if (result != VK_SUCCESS) {
		if (debugMode) {
			std::cout << "Failed to end recording compute command buffer! Error code: " << result << std::endl;
		}
		// Obs�u� b��d w razie potrzeby
	}

}

void GraphicsEngine::record_raytracing_command(VkCommandBuffer commandBuffer, uint32_t imageIndex) {

	// Przygotowanie beginInfo dla command buffera



	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.pNext = nullptr;
	beginInfo.flags = 0; // lub odpowiednie flagi, je�li potrzebujesz
	beginInfo.pInheritanceInfo = nullptr; // dla command buffer�w g��wnych

	VkResult result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
	if (result != VK_SUCCESS) {
		if (debugMode) {
			std::cout << "Failed to begin recording compute command buffer! Error code: " << result << std::endl;
		}
		// Mo�esz tutaj zako�czy� funkcj� lub podj�� inne dzia�anie
	}

	VkImageMemoryBarrier barrier = { };
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL; // lub inny oczekiwany layout
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = swapchainFrames[imageIndex].postProcessImage; // Twoja zmienna obrazu
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.srcAccessMask = 0; // poniewa� UNDEFINED nie wymaga synchronizacji
	barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT; // zale�y od dalszego u�ycia

	vkCmdPipelineBarrier(
		commandBuffer,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, // lub inny odpowiedni etap
		0, 0, NULL, 0, NULL, 1, &barrier
	);
	vkUtil::PipelineCacheChunk pipelineInfo = vkResources::scenePipelines->getPipeline("Ray");

	VkPhysicalDeviceRayTracingPipelinePropertiesKHR rtProps{};
	rtProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
	VkPhysicalDeviceProperties2 deviceProps2{};
	deviceProps2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
	deviceProps2.pNext = &rtProps;
	vkGetPhysicalDeviceProperties2(physicalDevice, &deviceProps2);

	// Dispatch zadania obliczeniowego
	const uint32_t handleSizeAligned = vkInit::alignedSize(rtProps.shaderGroupHandleSize,
		rtProps.shaderGroupHandleAlignment);

	
		

	VkStridedDeviceAddressRegionKHR raygenShaderSbtEntry{};
	raygenShaderSbtEntry.deviceAddress = vkUtil::getBufferDeviceAddress(device,raygenShaderBindingTable.buffer);
	raygenShaderSbtEntry.stride = handleSizeAligned;
	raygenShaderSbtEntry.size = handleSizeAligned;

	VkStridedDeviceAddressRegionKHR missShaderSbtEntry{};
	missShaderSbtEntry.deviceAddress = vkUtil::getBufferDeviceAddress(device, missShaderBindingTable.buffer);
	missShaderSbtEntry.stride = handleSizeAligned;
	missShaderSbtEntry.size = handleSizeAligned;

	VkStridedDeviceAddressRegionKHR hitShaderSbtEntry{};
	hitShaderSbtEntry.deviceAddress = vkUtil::getBufferDeviceAddress(device, hitShaderBindingTable.buffer);
	hitShaderSbtEntry.stride = handleSizeAligned;
	hitShaderSbtEntry.size = handleSizeAligned;

	VkStridedDeviceAddressRegionKHR callableShaderSbtEntry{};
	
	/*
		Dispatch the ray tracing commands
	*/
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipelineInfo.pipeline);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipelineInfo.pipelineLayout, 0, 1,
		&swapchainFrames[imageIndex].RayGenDescriptorSet, 0, 0);
	PFN_vkCmdTraceRaysKHR vkCmdTraceRaysKHR =
		(PFN_vkCmdTraceRaysKHR)vkGetDeviceProcAddr(device, "vkCmdTraceRaysKHR");
	vkCmdTraceRaysKHR(
		commandBuffer,
		&raygenShaderSbtEntry,
		&missShaderSbtEntry,
		&hitShaderSbtEntry,
		&callableShaderSbtEntry,
		1920,
		1080,
		1);

	/*
		Copy ray tracing output to swap chain image
	*/


	vkEndCommandBuffer(commandBuffer);
		// Obs�u� b��d w razie potrzeby
}


void GraphicsEngine::prepare_frame(uint32_t imageIndex) {
	vkUtil::SwapChainFrame& _frame = swapchainFrames[imageIndex];

	glm::vec3 eye = { -5.0f, 4.0f, 1.0f }; // sta�a pozycja kamery
	float angle = glfwGetTime(); // obr�t w czasie
	float distance = .5f ; // odleg�o�� na jak� patrzy kamera od swojego �rodka

	glm::vec3 direction = {
		cos(angle) * distance,
		0.0f,
		sin(angle) * distance
	};

	//glm::vec3 center = eye + direction;
	glm::vec3 center = { 0.0f, 4.0f, 0.0f };
	glm::vec3 up = { 0.0f, 1.0f, 0.0f };

	glm::mat4 view = glm::lookAt(eye, center, up);

	glm::mat4 projection = glm::perspective(glm::radians(45.0f), static_cast<float>(swapchainExtent.width) / static_cast<float>(swapchainExtent.height), 0.1f, 1024.0f);
	projection[1][1] *= -1;

	_frame.uboData.inverseProj = glm::inverse(projection);
	_frame.uboData.inverseView = glm::inverse(view);
	memcpy(_frame.uboDataWriteLocation, &(_frame.uboData), sizeof(vkUtil::UBO));

	_frame.lightData.position = glm::vec4(-2,4.5f,-1,1);
	_frame.lightData.intensity = glm::vec4(255);
	memcpy(_frame.lightDataWriteLocation, &(_frame.lightData), sizeof(vkUtil::Light));
	
	_frame.materialData.color = glm::vec3(0.1);
	_frame.materialData.shininess =  0.5f;
	_frame.materialData.ambientCoefficient = 0.35f;
	memcpy(_frame.materialDataWriteLocation, &(_frame.materialData), sizeof(vkUtil::Material));

	
	memcpy(_frame.normalsAndTextcordsDataWriteLocation, (accelerationStructure->inputArray.data()), accelerationStructure->totalExtraBLASBufferSize);
	memcpy(_frame.offsetsDataWriteLocation, (accelerationStructure->extraBLASoffsets.data()),  sizeof(float) * accelerationStructure->extraBLASoffsets.size());
	
	_frame.write_descriptors(accelerationStructure->topLevelAS.handle, bufferSize);
}