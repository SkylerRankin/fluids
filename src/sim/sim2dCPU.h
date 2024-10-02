#pragma once

#include <vector>
#include "glm/glm.hpp"

namespace FluidSim2dCPU {

	const char CELL_EMPTY = 0;
	const char CELL_SOLID = 1;
	const char CELL_FLUID = 2;

	class Solver {
	public:
		const float gravity = -9.81f;

		glm::ivec2 gridSize;
		float cellSize = 1.0f;
		std::vector<float> velocityU;
		std::vector<float> velocityV;
		std::vector<float> tempVelocityU;
		std::vector<float> tempVelocityV;
		std::vector<float> pressure;
		std::vector<float> objectScalar;
		std::vector<char> cellType;

		Solver(glm::ivec2 baseGridSize);
		void runStep();
		void advectVelocities();
		void applyExternalForces(float timestep);
		void calculatePressure(float timestep);
		void applyPressure();

		glm::vec2 traceParticle(glm::vec2 position, float backwardTimestep) const;
		float interpolateVelocityU(glm::vec2 position) const;
		float interpolateVelocityV(glm::vec2 position) const;
		float interpolatePressure(glm::vec2 position) const;
		float interpolateValue(glm::vec2 position, glm::vec2 cellOffset, const std::vector<float>& values) const;

		char getCellType(int x, int y);
		int countNeighboringMatchingCells(int x, int y, char matchingCellType);
		bool nextCellIntersectionOfType(glm::vec2 start, glm::vec2 direction, char type, glm::ivec2& intersectedCell, glm::vec2& intersection);
	};
}
