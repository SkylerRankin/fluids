#include <array>
#include "sim2dCPU.h"

FluidSim2dCPU::FluidSim2dCPU(glm::ivec2 gridSize) : gridSize(gridSize) {
	int totalCells = (gridSize.x + 2) * (gridSize.y + 2);
	velocityU = new float[totalCells];
	velocityV = new float[totalCells];
	pressure = new float[totalCells];
	objectScalar = new float[totalCells];
}

void FluidSim2dCPU::runStep() {
	
}