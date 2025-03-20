#pragma 
#include "config.h"
#include "GLFW/glfw3.h"
#include <View/graphicsEngine.h>
#ifndef APP_H
#define APP_H


class App
{
	
	GraphicsEngine* graphicsEngine;
	GLFWwindow* window;

	float lastFrame = 0.0f;
	double lastTime, currentTime;
	int numFrames;
	float frameTime;
	void build_glfw_window(int width, int height, bool debugMode);
	void calculateFrameRate();



	

public:
	App(int width,int height, bool debugMode);
	~App();
	void run();
	float deltaTime = 0.0f;
};



#endif // !APP_H

