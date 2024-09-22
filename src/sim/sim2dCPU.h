#pragma once

#include "glm/glm.hpp"

class FluidSim2dCPU {

public:
	FluidSim2dCPU(glm::ivec2 gridSize);
	void runStep();

private:
	glm::ivec2 gridSize;
	float* velocityU;
	float* velocityV;
	float* pressure;
	float* objectScalar;

};