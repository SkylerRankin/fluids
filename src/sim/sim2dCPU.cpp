#include <array>
#include <iostream>
#include <vector>

#include "sim2dCPU.h"
#include <Eigen/Sparse>


FluidSim2dCPU::Solver::Solver(glm::ivec2 gridSize) : gridSize(gridSize) {
	int totalCells = gridSize.x * gridSize.y;
	velocityU = std::vector<float>(totalCells, 0.0f);
	velocityV = std::vector<float>(totalCells, 0.0f);
	tempVelocityU = std::vector<float>(totalCells, 0.0f);
	tempVelocityV = std::vector<float>(totalCells, 0.0f);
	pressure = std::vector<float>(totalCells, 0.0f);
	objectScalar = std::vector<float>(totalCells, 0.0f);
	cellType = std::vector<char>(totalCells, 0);

	// Set all border cells to solid
	for (int x = 0; x < gridSize.x; x++) {
		cellType[x] = CELL_SOLID;
		cellType[(gridSize.y - 1) * gridSize.x + x] = CELL_SOLID;
	}
	for (int y = 0; y < gridSize.y; y++) {
		cellType[y * gridSize.x] = CELL_SOLID;
		cellType[y * gridSize.x + gridSize.x - 1] = CELL_SOLID;
	}

	cellType[gridSize.x + 1] = CELL_FLUID;
	cellType[gridSize.x + 2] = CELL_FLUID;
}

void FluidSim2dCPU::Solver::runStep() {
	float t = 1.0f;
	advectVelocities();
	applyExternalForces(t);
	calculatePressure(t);
	applyPressure();
}

void FluidSim2dCPU::Solver::advectVelocities() {
	float timestep = 1.0f;
	float halfCellSize = cellSize / 2.0f;

	for (int x = 0; x < gridSize.x; x++) {
		for (int y = 0; y < gridSize.y; y++) {
			if (getCellType(x, y) == CELL_SOLID ||
				getCellType(x, y) == CELL_EMPTY) {
				continue;
			}

			// TODO: what if the previous position lands in a solid or empty cell?

			// Compute new u component
			glm::vec2 position{ x * cellSize, y * cellSize + halfCellSize };
			glm::vec2 previousPosition = traceParticle(position, timestep);
			// TODO: previous position could be in solid or out of bounds. use the nearest point on the boundary
			tempVelocityU[y * gridSize.x + x] = interpolateVelocityU(previousPosition);

			// Compute new v component
			position = glm::vec2(x * cellSize + halfCellSize, y * cellSize);
			previousPosition = traceParticle(position, timestep);
			tempVelocityV[y * gridSize.x + x] = interpolateVelocityV(previousPosition);
		}
	}

	// Swap main and temporary velocity arrays
	velocityU.swap(tempVelocityU);
	velocityV.swap(tempVelocityV);
}

void FluidSim2dCPU::Solver::applyExternalForces(float timestep) {
	// Apply gravity to vertical velocity components
	for (int x = 0; x < gridSize.x; x++) {
		for (int y = 0; y < gridSize.y; y++) {
			int i = y * gridSize.x + x;
			if (cellType[i] != CELL_FLUID) {
				continue;
			}

			velocityV[i] += timestep * gravity;
		}
	}
}

void FluidSim2dCPU::Solver::calculatePressure(float timestep) {
	// A * P = B

	float density = 1.0f;
	float atmospherePressure = 1.0f;
	int n = gridSize.x * gridSize.y;
	
	// Construct the B matrix, based on velocity divergence
	std::vector<float> bValues;
	bValues.resize(n);
	for (int x = 0; x < gridSize.x; x++) {
		for (int y = 0; y < gridSize.y; y++) {
			int i = y * gridSize.x + x;
			if (cellType[i] == CELL_SOLID) continue;

			// Forward difference divergence, where velocities between fluid and solid cells are always 0.
			int scaleU0 = cellType[i] != CELL_SOLID;
			int scaleU1 = cellType[i + 1] != CELL_SOLID;
			int scaleV0 = cellType[i] != CELL_SOLID;
			int scaleV1 = cellType[i + gridSize.x] != CELL_SOLID;

			float divergence =
				velocityU[i + 1] * scaleU1 - velocityU[i] * scaleU0 +
				velocityV[i + gridSize.x] * scaleV1 - velocityV[i] * scaleV0;

			int emptyCells = countNeighboringMatchingCells(x, y, CELL_EMPTY);

			bValues[i] =
				density * cellSize * (1 / timestep)
				* divergence
				- emptyCells * atmospherePressure;
		}
	}
	Eigen::VectorXf b = Eigen::VectorXf::Map(bValues.data(), n);

	std::cout << b.size() << std::endl;
	for (int i = 0; i < n; i++) {
		std::cout << b[i] << ", ";
	}
	std::cout << std::endl;

	// Construct the A matrix, composed of coefficients related to each cell's neighboring cell types.
	std::vector<Eigen::Triplet<float>> aValues;
	for (int x = 0; x < gridSize.x; x++) {
		for (int y = 0; y < gridSize.y; y++) {
			int i = y * gridSize.x + x;
			if (cellType[i] != CELL_FLUID) continue;

			// Coefficient at aii is set to negative number of non-solid neighbors
			int emptyNeighors = countNeighboringMatchingCells(x, y, CELL_EMPTY);
			int solidNeighbors = countNeighboringMatchingCells(x, y, CELL_SOLID);
			aValues.push_back({ i, i, -1.0f * (emptyNeighors + solidNeighbors) });

			// Set the coefficient for the cell at x - 1
			if (x > 0 && cellType[i - 1] == CELL_FLUID) {
				aValues.push_back({ i, i - 1, 1 });
			}

			// Set the coefficient for the cell at x + 1
			if (x < gridSize.x - 1 && cellType[i + 1] == CELL_FLUID) {
				aValues.push_back({ i, i + 1, 1 });
			}

			// Set the coefficient for the cell at y - 1
			if (y < 0 && cellType[i - gridSize.x] == CELL_FLUID) {
				aValues.push_back({ i, i - gridSize.x, 1 });
			}

			// Set the coefficient for the cell at x + 1
			if (y < gridSize.y - 1 && cellType[i + gridSize.x] == CELL_FLUID) {
				aValues.push_back({ i, i + gridSize.x, 1 });
			}
		}
	}
	Eigen::SparseMatrix<float> a{n, n};
	a.setZero();
	a.setFromTriplets(aValues.begin(), aValues.end());
	std::cout << Eigen::MatrixXf(a) << std::endl;

	// TODO: try other solvers listed here: https://eigen.tuxfamily.org/dox/group__TopicSparseSystems.html
	//Eigen::SimplicialLDLT<Eigen::SparseMatrix<float>> solver;
	//solver.compute(a);

	//if (solver.info() != Eigen::Success) {
	//	std::cout << "Matrix decomposition failed (" << solver.info() << ")." << std::endl;
	//	return;
	//}

	//Eigen::VectorXf p = solver.solve(b);
	//if (solver.info() != Eigen::Success) {
	//	std::cout << "Matrix solving failed (" << solver.info() << ")." << std::endl;
	//	return;
	//}

	//for (int i = 0; i < n; i++) {
	//	pressure[i] = p[i];
	//}
}

void FluidSim2dCPU::Solver::applyPressure() {}

glm::vec2 FluidSim2dCPU::Solver::traceParticle(glm::vec2 position, float timestep) const {
	// Use RK2 interpolation on vector field to approximate particle's previous position.

	glm::vec2 velocity{
		interpolateVelocityU(position),
		interpolateVelocityV(position)
	};
	velocity = glm::vec2(
		interpolateVelocityU(position - (velocity * timestep / 2.0f)),
		interpolateVelocityV(position - (velocity * timestep / 2.0f))
	);

	return position - timestep * velocity;
}

float FluidSim2dCPU::Solver::interpolateVelocityU(glm::vec2 position) const {
	return interpolateValue(position, glm::vec2(0.0f, cellSize / 2.0f), velocityU);
}

float FluidSim2dCPU::Solver::interpolateVelocityV(glm::vec2 position) const {
	return interpolateValue(position, glm::vec2(cellSize / 2.0f, 0.0f), velocityV);
}

float FluidSim2dCPU::Solver::interpolatePressure(glm::vec2 position) const {
	return interpolateValue(position, glm::vec2(0.0f), pressure);
}

inline float FluidSim2dCPU::Solver::interpolateValue(glm::vec2 position, glm::vec2 cellOffset, const std::vector<float>& values) const {
	// Grid positions of each corner in the sampled region
	float x0 = floor(position.x / cellSize - cellOffset.x) + cellOffset.x;
	float x1 = x0 + 1;
	float y0 = floor(position.y / cellSize - cellOffset.y) + cellOffset.y;
	float y1 = y0 + 1;

	// Scalars used to zero out corners that are out of bounds
	int mx0 = x0 >= 0 && x0 < gridSize.x;
	int mx1 = x1 >= 0 && x1 < gridSize.x;
	int my0 = y0 >= 0 && y0 < gridSize.y;
	int my1 = y1 >= 0 && y1 < gridSize.y;

	// Area of rectangle formed at each corner.
	float a00 = (position.x - x0) * (position.y - y0) * mx1 * my1;
	float a01 = (position.x - x0) * (y1 - position.y) * mx1 * my0;
	float a10 = (x1 - position.x) * (position.y - y0) * mx0 * my1;
	float a11 = (x1 - position.x) * (y1 - position.y) * mx0 * my0;
	float totalArea = a00 + a01 + a10 + a11;

	// Scale areas by total area to ignore out of bounds corners
	a00 /= totalArea;
	a01 /= totalArea;
	a10 /= totalArea;
	a11 /= totalArea;

	// Offset corners so that indices align with array access, rather than being staggered by a half cell in one direction.
	x0 -= cellOffset.x;
	x1 -= cellOffset.x;
	y0 -= cellOffset.y;
	y1 -= cellOffset.y;

	// Compute the bilinear interpolation
	return
		a11 * values.data()[(int) floor(y0 * gridSize.x + x0)] +
		a01 * values.data()[(int) floor(y0 * gridSize.x + x1)] +
		a10 * values.data()[(int) floor(y1 * gridSize.x + x0)] +
		a00 * values.data()[(int) floor(y1 * gridSize.x + x1)];
}

char FluidSim2dCPU::Solver::getCellType(int x, int y) {
	return cellType[y * gridSize.x + x];
}

int FluidSim2dCPU::Solver::countNeighboringMatchingCells(int x, int y, char matchingCellType) {
	int i = y * gridSize.x + x;
	int count = 0;
	if (x >= 0 && cellType[i - 1] == matchingCellType) count++;
	if (x < gridSize.x - 1 && cellType[i + 1] == matchingCellType) count++;
	if (y >= 0 && cellType[i - gridSize.x] == matchingCellType) count++;
	if (y < gridSize.y - 1 && cellType[i + gridSize.x] == matchingCellType) count++;
	return count;
}

bool FluidSim2dCPU::Solver::nextCellIntersectionOfType(glm::vec2 start, glm::vec2 direction, char type, glm::ivec2& intersectedCell, glm::vec2& intersection) {
	direction = glm::normalize(direction);
	float angle = glm::acos(direction.x / glm::length(direction));
	glm::ivec2 currentCell{ glm::floor(start / cellSize) };
	glm::vec2 step{ glm::sign(direction.x), glm::sign(direction.y) };
	glm::vec2 offset{
		direction.x > 0 ? glm::ceil(start.x) - start.x : start.x - glm::floor(start.x),
		direction.y > 0 ? glm::ceil(start.y) - start.y : start.y - glm::floor(start.y)
	};
	
	// Handle cases of axis-aligned directions
	if (direction.x == 0 || direction.y == 0) {
		while (true) {
			currentCell += direction;
			int i = currentCell.y * gridSize.x + currentCell.x;
			if (i < 0 || i >= gridSize.x * gridSize.y) {
				return false;
			} else if (cellType[i] == type) {
				break;
			}
		}

		intersectedCell.x = currentCell.x;
		intersectedCell.y = currentCell.y;

		if (direction.x == 0) {
			intersection.x = start.x;
			intersection.y = currentCell.y * cellSize;
		} else {
			intersection.x = currentCell.x * cellSize;
			intersection.y = start.y;
		}

		return true;
	}

	// TODO: handle case of divide by zero
	glm::vec2 tMax{
		offset.x / glm::cos(angle),
		offset.y / glm::sin(angle)
	};
	glm::vec2 tDelta{
		cellSize / glm::cos(angle),
		cellSize / glm::sin(angle)
	};

	while (true) {
		if (tMax.x < tMax.y) {
			tMax.x += tDelta.x;
			currentCell.x += static_cast<int>(step.x);
		} else {
			tMax.y += tDelta.y;
			currentCell.y += static_cast<int>(step.y);
		}

		int i = currentCell.y * gridSize.x + currentCell.x;
		if (i < 0 || i >= gridSize.x * gridSize.y) {
			return false;
		} else if (cellType[i] == type) {
			break;
		}
	}

	intersectedCell.x = currentCell.x;
	intersectedCell.y = currentCell.y;

	// Finds the specific location along the edge of the intersected cell. Finds the two lines bordering
	// the final intersected cell, and takes the intersection point of the two which is farther from the
	// start position.
	float m = direction.y / direction.x;
	glm::vec2 c{
		intersectedCell.x + (direction.x > 0 ? 0 : 1) * cellSize,
		intersectedCell.y + (direction.y > 0 ? 0 : 1) * cellSize,
	};

	glm::vec2 ix{ c.x, m * (c.x - start.x) + start.y };
	glm::vec2 iy{ (c.y - start.y) / m + start.x, c.y };

	if (glm::length(ix - start) > glm::length(iy - start)) {
		intersection.x = ix.x;
		intersection.y = ix.y;
	} else {
		intersection.x = iy.x;
		intersection.y = iy.y;
	}

	return true;
}
