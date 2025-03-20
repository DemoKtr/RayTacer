#pragma once

#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include "config.h"
#include <vector>
#include "View/vkUtil/frame.h"




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



	std::vector<vkUtil::SwapChainFrame> swapchainFrames;

	void make_instance(); //instance Setup
	void choice_device();
	void create_swapchain();
	void recreate_swapchain();
	void cleanup_swapchain();
	void create_pipeline();
	void finalize_setup();
	void make_assets();
	void create_frame_command_buffer();
	void create_frame_resources(int number_of_models);
	void create_descriptor_set_layouts();
	


	void record_draw_command();
	void record_unlit_draw_command();
	void record_pbr_draw_command();

	void prepare_scene();
	void render_objects();

	void prepare_frame();

	void build_accelerationStructures();
	
	
public:
	GraphicsEngine(GLFWwindow* window, int width, int height, bool debugMode);
	void render();
	void InitImGui(GLFWwindow* window);
	~GraphicsEngine();
};