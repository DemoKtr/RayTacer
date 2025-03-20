#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "View/graphicsEngine.h"
#include "View/vkInit/instance.h"
#include "View/vkInit/devices.h"
#include "View/vkUtil/queues.h"
#include <View/vkInit/swapchain.h>
#include <View/vkUtil/Pipelines/pipelineCache.h>
#include <View/vkUtil/Pipelines/computePipelineBuilder.h>
#include <View/vkInit/descriptors.h>



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
	create_swapchain();
	create_descriptor_set_layouts();
	create_pipeline();

	finalize_setup();


	make_assets();

}

GraphicsEngine::~GraphicsEngine() {

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
		frame.device = device;
		//frame.physicalDevice = physicalDevice;
		//frame.width = swapchainExtent.width;
		//frame.height = swapchainExtent.height;
		//frame.make_depth_resources();

	}
	frameNumber = 0;
	std::cout << maxFramesInFlight << std::endl;
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
	/*
	pipelineBuilder.set_overwrite_mode(true);
	pipelineBuilder.specify_vertex_format(
		vkMesh::getVertexInputBindingDescription(),
		vkMesh::getVertexInputAttributeDescription());
	pipelineBuilder.specify_vertex_shader("resources/shaders/vert.spv");
	pipelineBuilder.specify_fragment_shader("resources/shaders/frag.spv");
	pipelineBuilder.specify_swapchain_extent(swapchainExtent);
	pipelineBuilder.clear_depth_attachment();
	pipelineBuilder.set_color_blending(false);
	pipelineBuilder.add_descriptor_set_layout(postprocessDescriptorSetLayout);
	pipelineBuilder.add_descriptor_set_layout(textureDescriptorSetLayout);
	pipelineBuilder.use_depth_test(true);
	pipelineBuilder.setPushConstants();
	pipelineBuilder.dynamicRendering = true;
	vkUtil::GraphicsPipelineOutBundle output = pipelineBuilder.build(vk::Format::eR32G32B32A32Sfloat, swapchainFrames[0].depthFormat);

	pipeline.pipelineLayout = output.layout;
	pipeline.pipeline = output.pipeline;

	vkResources::scenePipelines->addPipeline("Unlit Pipeline", pipeline);
	pipelineBuilder.reset();
	*/
	/*
	vkInit::ComputePipelineBuilder computePipelineBuilder(device);
	computePipelineBuilder.add_descriptor_set_layout();
	computePipelineBuilder.set_push_constant();
	computePipelineBuilder.specify_compute_shader();
	vkUtil::GraphicsPipelineOutBundle output = computePipelineBuilder.build(true);
	pipeline.pipelineLayout = output.layout;
	pipeline.pipeline = output.pipeline;

	vkResources::scenePipelines->addPipeline("Unlit Pipeline", pipeline);
	pipelineBuilder.reset();
	*/
	
}

void GraphicsEngine::finalize_setup() {

}

void GraphicsEngine::make_assets() {

}


void GraphicsEngine::create_frame_command_buffer() {
	/*
	CommandPool = vkInit::make_command_pool(physicalDevice, device, surface, debugMode);
	computeCommandPool = vkInit::make_compute_command_pool(physicalDevice, device, surface, debugMode);
	transferCommandPool = vkInit::make_transfer_command_pool(physicalDevice, device, surface, debugMode);
	vkInit::commandBufferInputChunk commandBufferInput = { device,CommandPool, swapchainFrames };
	maincommandBuffer = vkInit::make_command_buffer(commandBufferInput, debugMode);
	vkInit::make_imgui_frame_command_buffers(commandBufferInput, debugMode);

	commandBufferInput.commandPool = computeCommandPool;
	computeCommandBuffer = vkInit::make_command_buffer(commandBufferInput, debugMode);
	commandBufferInput.commandPool = transferCommandPool;
	transferCommandBuffer = vkInit::make_command_buffer(commandBufferInput, debugMode);
	*/
}

void GraphicsEngine::create_frame_resources(int number_of_models) {
	/*
	vkInit::descriptorSetLayoutData bindings;
	bindings.count = 2;
	bindings.types.push_back(vk::DescriptorType::eUniformBuffer);
	bindings.types.push_back(vk::DescriptorType::eStorageBuffer);
	DescriptorPool = vkInit::make_descriptor_pool(device, static_cast<uint32_t>(swapchainFrames.size()), bindings);
	

	for (vkUtil::SwapChainFrame& frame : swapchainFrames) //referencja 
	{
		frame.imageAvailable = vkInit::make_semaphore(device, debugMode);
		frame.renderFinished = vkInit::make_semaphore(device, debugMode);
		frame.computeFinished = vkInit::make_semaphore(device, debugMode);
		frame.inFlight = vkInit::make_fence(device, debugMode);
		frame.make_descriptors_resources(number_of_models);
		frame.DescriptorSet = vkInit::allocate_descriptor_set(device, DescriptorPool, DescriptorSetLayout);


	}
	*/
}

void GraphicsEngine::create_descriptor_set_layouts() {
	/*
	vkInit::descriptorSetLayoutData bindings;
	bindings.count = 2;
	bindings.indices.push_back(0);
	bindings.types.push_back(vk::DescriptorType::eUniformBuffer);
	bindings.counts.push_back(1);
	bindings.stages.push_back(vk::ShaderStageFlagBits::eVertex);

	bindings.indices.push_back(1);
	bindings.types.push_back(vk::DescriptorType::eStorageBuffer);
	bindings.counts.push_back(1);
	bindings.stages.push_back(vk::ShaderStageFlagBits::eVertex);

	postprocessDescriptorSetLayout = vkInit::make_descriptor_set_layout(device, bindings);
	*/
}
