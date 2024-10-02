#include <format>
#include "glm/glm.hpp"
#include "../acutest.h"

#include "../../src/sim/sim2dCPU.h"

const float epsilon = 0.00001f;

void CHECK_FLOAT(float expected, float actual) {
    TEST_CHECK(abs(actual - expected) < epsilon);
    TEST_MSG(std::format("Expected {}, got {}.", expected, actual).c_str());
}

void CHECK_VEC2(glm::vec2 expected, glm::vec2 actual) {
    TEST_CHECK(abs(actual.x - expected.x) < epsilon);
    TEST_CHECK(abs(actual.y - expected.y) < epsilon);
    TEST_MSG(std::format("Expected ({}, {}), got ({}, {}).", expected.x, expected.y, actual.x, actual.y).c_str());
}

void CHECK_IVEC2(glm::ivec2 expected, glm::ivec2 actual) {
    TEST_CHECK(abs(actual.x - expected.x) < epsilon);
    TEST_CHECK(abs(actual.y - expected.y) < epsilon);
    TEST_MSG(std::format("Expected ({}, {}), got ({}, {}).", expected.x, expected.y, actual.x, actual.y).c_str());
}

void interpolateValue_normalUV() {
    FluidSim2dCPU::Solver sim{ glm::ivec2(4, 4) };
    sim.cellSize = 1.0f;

    glm::vec2 velocities[] = {
        glm::vec2(1.9f, 7.6f), glm::vec2(8.9f, 2.2f), glm::vec2(0.0f, 0.0f), glm::vec2(0.0f, 0.0f),
        glm::vec2(0.0f, 0.0f), glm::vec2(8.1f, 0.0f), glm::vec2(17.4f, 0.0f), glm::vec2(0.0f, 0.0f),
        glm::vec2(0.0f, 0.0f), glm::vec2(18.3f, 4.5f), glm::vec2(11.7f, 2.8f), glm::vec2(0.0f, 0.0f),
        glm::vec2(0.0f, 0.0f), glm::vec2(0.0f, 1.1f), glm::vec2(0.0f, 6.2f), glm::vec2(0.0f, 0.0f)
    };

    for (int i = 0; i < 16; i++) {
        sim.velocityU[i] = velocities[i].x;
        sim.velocityV[i] = velocities[i].y;
    }

    // [18.3]          [11.7]
    //   .        (P)     .
    //   .                .
    //   .                .
    // [8.1]           [17.4]
    //
    // x00: (1, 1.5), area = (1.8 - 1) * (2.3 - 1.5) = 0.64
    // x01: (1, 2.5), area = (1.8 - 1) * (2.5 - 2.3) = 0.16
    // x10: (2, 1.5), area = (2 - 1.8) * (2.3 - 1.5) = 0.16
    // x11: (2, 2.5), area = (2 - 1.8) * (2.5 - 2.3) = 0.04
    // Interpolated value = 8.1 * 0.04 + 17.4 * 0.16 + 18.3 * 0.16 + 11.7 * 0.64 = 13.524

    float expected = 13.524f;
    float result = sim.interpolateVelocityU(glm::vec2(1.8f, 2.3f));
    CHECK_FLOAT(expected, result);

    // [1.1]          [6.2]
    //   .               .
    //   .               .
    //   .    (P)        .
    // [4.5]           [2.8]
    //
    // x00: (1.5, 2), area = (1.8 - 1.5) * (2.3 - 2) = 0.09
    // x01: (1.5, 3), area = (1.8 - 1.5) * (3 - 2.3) = 0.21
    // x10: (2.5, 2), area = (2.5 - 1.8) * (2.3 - 2) = 0.21
    // x11: (2.5, 3), area = (2.5 - 1.8) * (3 - 2.3) = 0.49
    // Interpolated value = 0.09 * 6.2 + 0.21 * 2.8 + 0.21 * 1.1 + 0.49 * 4.5 = 3.582

    expected = 3.582f;
    result = sim.interpolateVelocityV(glm::vec2(1.8f, 2.3f));
    CHECK_FLOAT(expected, result);

    // [1.1](P)        [6.2]
    //   .               .
    //   .               .
    //   .               .
    // [4.5]           [2.8]
    //
    // x00: (1.5, 2), area = (1.5 - 1.5) * (3 - 2) = 0
    // x01: (1.5, 3), area = (1.5 - 1.5) * (3 - 3) = 0
    // x10: (2.5, 2), area = (2.5 - 1.5) * (3 - 2) = 1
    // x11: (2.5, 3), area = (2.5 - 1.5) * (3 - 3) = 0
    // Interpolated value = 4.5 * 0 + 2.8 * 0 + 1.1 * 1 + 6.2 * 0 = 1.1

    expected = 1.1f;
    result = sim.interpolateVelocityV(glm::vec2(1.5f, 3.0f));
    CHECK_FLOAT(expected, result);
}

void interpolateValue_borderUV() {
    FluidSim2dCPU::Solver sim{ glm::ivec2(4, 4) };
    sim.cellSize = 1.0f;

    glm::vec2 velocities[] = {
        glm::vec2(1.9f, 13.3f), glm::vec2(8.9f, 2.2f), glm::vec2(0.0f, 0.0f), glm::vec2(0.0f, 13.3f),
        glm::vec2(0.0f, 5.1f), glm::vec2(8.1f, 0.0f), glm::vec2(17.4f, 0.0f), glm::vec2(0.0f, 5.1f),
        glm::vec2(0.0f, 0.0f), glm::vec2(18.3f, 4.5f), glm::vec2(11.7f, 2.8f), glm::vec2(3.9f, 0.0f),
        glm::vec2(0.0f, 0.0f), glm::vec2(0.0f, 1.1f), glm::vec2(0.0f, 6.2f), glm::vec2(14.1f, 0.0f)
    };

    for (int i = 0; i < 16; i++) {
        sim.velocityU[i] = velocities[i].x;
        sim.velocityV[i] = velocities[i].y;
    }

    // [1.9]           [8.9]
    //   .        (P)    .
    //   .               .
    //   .               .
    // [   ]           [   ]
    //
    // x00: (0, -0.5), area = (0.6 - 0) * (0.2 - -0.5) = 0.42
    // x01:  (0, 0.5), area = (0.6 - 0) * (0.5 - 0.2) = 0.18
    // x10: (1, -0.5), area = (1 - 0.6) * (0.2 - -0.5) = 0.28
    // x11:  (1, 0.5), area = (1 - 0.6) * (0.5 - 0.2) = 0.12
    // Interpolated value = 1.9 * 0.28 / (0.28 + 0.42) + 8.9 * 0.42 / (0.28 + 0.42) = 6.1

    float expected = 6.1f;
    float result = sim.interpolateVelocityU(glm::vec2(0.6f, 0.2f));
    CHECK_FLOAT(expected, result);

    // [   ]           [5.1]
    //   .               .
    //   .               .
    //   .       (P)     .
    // [   ]           [13.3]
    //
    // x00: (-0.5, 0), area = (0.15 - -0.5) * (0.4 - 0) = 0.26
    // x01: (-0.5, 1), area = (0.15 - -0.5) * (1 - 0.4) = 0.39
    // x10:  (0.5, 0), area = (0.5 - 0.15) * (0.4 - 0) = 0.14
    // x11:  (0.5, 1), area = (0.5 - 0.15) * (1 - 0.4) = 0.21
    // Interpolated value = 5.1 * 0.26 / (0.26 + 0.39) + 13.3 * 0.39 / (0.26 + 0.39)

    expected = 10.02f;
    result = sim.interpolateVelocityV(glm::vec2(0.15f, 0.4f));
    CHECK_FLOAT(expected, result);

    // [   ]           [   ]
    //   .    (P)        .
    //   .               .
    //   .               .
    // [1.1]           [6.2]
    //
    // x00: (1.5, 3), area = (1.8 - 1.5) * (3.7 - 3) = 0.21
    // x01: (1.5, 4), area = (1.8 - 1.5) * (4 - 3.7) = 0.09
    // x10: (2.5, 3), area = (2.5 - 1.8) * (3.7 - 3) = 0.49
    // x11: (2.5, 4), area = (2.5 - 1.8) * (4 - 3.7) = 0.21
    // Interpolated value = 1.1 * 0.21 / (0.21 + 0.09) + 6.2 * 0.09 / (0.21 + 0.09) = 2.63

    expected = 2.63f;
    result = sim.interpolateVelocityV(glm::vec2(1.8f, 3.7));
    CHECK_FLOAT(expected, result);

    // [14.1]          [   ]
    //   .               .
    //   .               .
    //   .          (P)  .
    // [3.9]           [   ]
    //
    // x00: (3, 2.5), area = (3.9 - 3) * (2.7 - 2.5) = 0.18
    // x01: (3, 3.5), area = (3.9 - 3) * (3.5 - 2.7) = 0.72
    // x10: (4, 2.5), area = (4 - 3.9) * (2.7 - 2.5) = 0.02
    // x11: (4, 3.5), area = (4 - 3.9) * (3.5 - 2.7) = 0.08
    // Interpolated value = 3.9 * 0.08 / (0.08 + 0.02) + 14.1 * 0.02 / (0.08 + 0.02) = 5.94

    expected = 5.94f;
    result = sim.interpolateVelocityU(glm::vec2(3.9, 2.7));
    CHECK_FLOAT(expected, result);
}

void interpolateValue_normalPressure() {
    // TODO
}

void nextCellIntersectionOfType() {
    FluidSim2dCPU::Solver sim{ glm::vec2(4, 4) };

    for (int i = 0; i < 16; i++) {
        sim.cellType[i] = FluidSim2dCPU::CELL_EMPTY;
    }

    sim.cellType[4] = FluidSim2dCPU::CELL_FLUID;
    sim.cellType[7] = FluidSim2dCPU::CELL_SOLID;
    sim.cellType[13] = FluidSim2dCPU::CELL_FLUID;

    //  [E] [F] [E] [E]
    //  [E] [E] [E] [E]
    //  [F] [E] [E] [S]
    //  [E] [E] [E] [E]

    glm::ivec2 intersectedCell;
    glm::vec2 intersection;
    bool hit;

    // Normal intersection in positive direction
    hit = sim.nextCellIntersectionOfType(
        glm::vec2(0.5f, 0.5f),
        glm::vec2(3, 1),
        FluidSim2dCPU::CELL_SOLID,
        intersectedCell,
        intersection);
    TEST_CHECK(hit);
    CHECK_IVEC2(glm::ivec2(3, 1), intersectedCell);
    CHECK_VEC2(glm::vec2(3.0f, 1.33333f), intersection);

    // Normal intersection in negative x direction, hits right side of cell
    hit = sim.nextCellIntersectionOfType(
        glm::vec2(2.0f, 0.0f),
        glm::vec2(-1.3f, 2.2f),
        FluidSim2dCPU::CELL_FLUID,
        intersectedCell,
        intersection);
    TEST_CHECK(hit);
    CHECK_IVEC2(glm::ivec2(0, 1), intersectedCell);
    CHECK_VEC2(glm::vec2(1.0f, 1.69231), intersection);

    // Normal intersection in negative x direction, hits bottom side of cell
    hit = sim.nextCellIntersectionOfType(
        glm::vec2(2.0f, 0.0f),
        glm::vec2(-2.2f, 1.3f),
        FluidSim2dCPU::CELL_FLUID,
        intersectedCell,
        intersection);
    TEST_CHECK(hit);
    CHECK_IVEC2(glm::ivec2(0, 1), intersectedCell);
    CHECK_VEC2(glm::vec2(0.30769f, 1.0f), intersection);

    // Normal intersection in negative y direction
    hit = sim.nextCellIntersectionOfType(
        glm::vec2(0.5f, 3.5f),
        glm::vec2(3.5f, -1.75f),
        FluidSim2dCPU::CELL_SOLID,
        intersectedCell,
        intersection);
    TEST_CHECK(hit);
    CHECK_IVEC2(glm::ivec2(3, 1), intersectedCell);
    CHECK_VEC2(glm::vec2(3.5f, 2.0f), intersection);

    // Normal intersection in negative y direction
    hit = sim.nextCellIntersectionOfType(
        glm::vec2(0.5f, 3.5f),
        glm::vec2(3.5f, -2.8f),
        FluidSim2dCPU::CELL_SOLID,
        intersectedCell,
        intersection);
    TEST_CHECK(hit);
    CHECK_IVEC2(glm::ivec2(3, 1), intersectedCell);
    CHECK_VEC2(glm::vec2(3, 1.5f), intersection);

    // Straight right intersection
    hit = sim.nextCellIntersectionOfType(
        glm::vec2(0.4f, 1.3f),
        glm::vec2(1, 0),
        FluidSim2dCPU::CELL_SOLID,
        intersectedCell,
        intersection);
    TEST_CHECK(hit);
    CHECK_IVEC2(glm::ivec2(3, 1), intersectedCell);
    CHECK_VEC2(glm::vec2(3, 1.3f), intersection);

    // Straight up intersection
    hit = sim.nextCellIntersectionOfType(
        glm::vec2(1.4f, 0.3f),
        glm::vec2(0, 1.5f),
        FluidSim2dCPU::CELL_FLUID,
        intersectedCell,
        intersection);
    TEST_CHECK(hit);
    CHECK_IVEC2(glm::ivec2(1, 3), intersectedCell);
    CHECK_VEC2(glm::vec2(1.4f, 3.0f), intersection);

    // No intersection
    hit = sim.nextCellIntersectionOfType(
        glm::vec2(0.5f, 1.5f),
        glm::vec2(3, 1),
        FluidSim2dCPU::CELL_SOLID,
        intersectedCell,
        intersection);
    TEST_CHECK(!hit);
}

TEST_LIST = {
    { "sim2dCPU::interpolateValue normal UV", interpolateValue_normalUV },
    { "sim2dCPU::interpolateValue border UV", interpolateValue_borderUV },
    { "sim2dCPU::nextCellIntersectionOfType", nextCellIntersectionOfType },
    { nullptr, nullptr }
};
