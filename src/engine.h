#pragma once
#include "glCommon.h"
#include "renderer/renderer2d.h"

class Engine {
public:
	Engine(GLFWwindow* window, const std::string& executableDirectory);
	~Engine();
	void keyCallback(int key, int scanCode, int action, int mods);
	void mousePositionCallback(double x, double y);
	void mouseEnteredCallback(int entered);
	void renderFrame();
	std::string getExecutableDirectory();

private:
	GLFWwindow* window;
	FluidRenderer2D renderer2d;
	double previousFrameTime;
	glm::ivec2 previousScreenSize;
	std::string executableDirectory;
};
