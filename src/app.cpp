#include "app.h"

void App::build_glfw_window(int width, int height, bool debugMode) {
	glfwInit();

	//no default rendering client, we'll hook vulkan up
	//to the window later
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	//resizing breaks the swapchain, we'll disable it for now
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	//GLFWwindow* glfwCreateWindow (int width, int height, const char *title, GLFWmonitor *monitor, GLFWwindow *share)
	if (window = glfwCreateWindow(width, height, "VulkanGame", nullptr, nullptr)) {
		if (debugMode) {
			std::cout << "Successfully made a glfw window , width: " << width << ", height: " << height << '\n';
		}
	}
	else {
		if (debugMode) {
			std::cout << "GLFW window creation failed\n";
		}
	}
	glfwSetWindowUserPointer(window, this);
	//glfwSetCursorPosCallback(window, mouse_callback);
	//glfwSetScrollCallback(window, scroll_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	//glfwSetKeyCallback(window, keyCallback);
	//glfwSetMouseButtonCallback(window, mouseButtonCallback);
}

App::App(int width, int height, bool debugMode) {
	build_glfw_window(width,height, debugMode);

	double startTime = glfwGetTime();

	//scene = new Scene();


	graphicsEngine = new GraphicsEngine(window, width,height, debugMode);

	//graphicsEngine->InitImGui(window);
	double endTime = glfwGetTime();
	double delta = endTime - startTime;

	std::cout << "Time to Init GraphicEngine: " << delta << std::endl;
}

App::~App() {
	delete graphicsEngine;
}

void App::run() {
}
