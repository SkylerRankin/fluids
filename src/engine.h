#pragma once
#include "glCommon.h"
#include "renderer/renderer2d.h"

class Engine {
public:
	Engine(GLFWwindow* window);
	~Engine();
	void keyCallback(int key, int scanCode, int action, int mods);
	void mousePositionCallback(double x, double y);
	void mouseEnteredCallback(int entered);
	void renderFrame();

private:
	GLFWwindow* window;
	FluidRenderer2D renderer2d;
	double previousFrameTime;
	glm::ivec2 previousScreenSize;
};