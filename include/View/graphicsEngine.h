#pragma once

#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include "config.h"
#include <vector>
#include "View/vkUtil/frame.h"
#include "View/vkAccelerationStructures/AccelerationStructureMenagerie.h"



class GraphicsEngine {
	int width,height;
	const char* appName{ "RayTracer" };


	//synchronizers 
	int maxFramesInFlight, frameNumber;

	float fullTime = 0.0f;

	GLFWwindow* mainWindow;
	bool debugMode{true};
	VkInstance instance{ nullptr }; //instancja
	//debug callback	
	VkDebugUtilsMessengerEXT debugMessenger{ nullptr };
	
	//surface
	VkSurfaceKHR surface;
	//physical device
	VkPhysicalDevice physicalDevice{ nullptr };
	//logical device
	VkDevice device{ nullptr };
	//queues
	VkQueue graphicsQueue{ nullptr };
	VkQueue presentQueue{ nullptr };
	VkQueue computeQueue{ nullptr };
	VkQueue transferQueue{ nullptr };
	//swapchain
	VkSwapchainKHR swapchain;

	//swapchainFormat
	VkFormat swapchainFormat;
	//swapchainExtent
	VkExtent2D swapchainExtent;

	//commandBuffers
	VkCommandBuffer maincommandBuffer;
	VkCommandBuffer transferCommandBuffer;
	VkCommandBuffer computeCommandBuffer;
	VkCommandPool CommandPool;
	VkCommandPool computeCommandPool;
	VkCommandPool transferCommandPool;


	VkDescriptorPool rayCastDescriptorPool;
	VkDescriptorPool finalImageDescriptorPool;

	VkDescriptorSetLayout rayCastDescriptorSetLayout;
	VkDescriptorSetLayout finalImageDescriptorSetLayout;

	VkDescriptorPool rayGenDescriptorPool;
	VkDescriptorSetLayout rayGenDescriptorSetLayout;

	std::vector<vkUtil::SwapChainFrame> swapchainFrames;


	vkAccelerationStructure::VertexMenagerie *accelerationStructure;

	void make_instance(); //instance Setup
	void choice_device();
	void create_swapchain();
	void recreate_swapchain();
	void cleanup_swapchain();
	void create_pipeline();
	void finalize_setup();
	void make_assets();
	void create_frame_command_buffer();
	void create_frame_resources();
	void create_descriptor_set_layouts();
	


	void record_draw_command(VkCommandBuffer commandBuffer, uint32_t imageIndex);
	void record_compute_command(VkCommandBuffer commandBuffer, uint32_t imageIndex);
	

	void prepare_scene();
	void render_objects();

	void prepare_frame(uint32_t imageIndex);

	void build_accelerationStructures();
	
	
public:
	GraphicsEngine(GLFWwindow* window, int width, int height, bool debugMode);
	void render();
	void InitImGui(GLFWwindow* window);
	~GraphicsEngine();
};