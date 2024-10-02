#include <filesystem>
#include <iostream>
#include <vector>

#include "renderer2d.h"
#include "../glCommon.h"
#include "../camera.h"

FluidRenderer2D::FluidRenderer2D() {
	
}

void FluidRenderer2D::initialize(const glm::ivec2 screenSize, std::string executableDirectory) {
	glGenVertexArrays(1, &gridVAO);
	glBindVertexArray(gridVAO);

	const int vertexPositionLocation = 0;
	const int vertexUVLocation = 1;

	glGenBuffers(1, &gridVBO);
	glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
	updateGridQuad(screenSize);
	glVertexAttribPointer(vertexPositionLocation, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GL_FLOAT), 0);
	glEnableVertexAttribArray(vertexPositionLocation);
	glVertexAttribPointer(vertexUVLocation, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GL_FLOAT), (GLvoid*)(3 * sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(vertexUVLocation);

	glGenTextures(1, &gridTexture);
	glBindTexture(GL_TEXTURE_2D, gridTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, gridSize.x, gridSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	gridProgram = glCreateProgram();
	gridFragmentShader.setExecutableDirectory(executableDirectory);
	gridVertexShader.setExecutableDirectory(executableDirectory);
	gridFragmentShader.compileAndAttach(gridProgram, GL_FRAGMENT_SHADER, "noop_fragment.glsl");
	gridVertexShader.compileAndAttach(gridProgram, GL_VERTEX_SHADER, "noop_vertex.glsl");
	glLinkProgram(gridProgram);

	debugImageData.resize(gridSize.x * gridSize.y * 4);
	gridTextureData.resize(gridSize.x * gridSize.y * 4);
}

void FluidRenderer2D::screenSizeUpdate(const glm::ivec2 newSize) {
	updateGridQuad(newSize);
}

void FluidRenderer2D::render(GLFWwindow* window, float aspectRatio, float elapsedSeconds) {
	fluidSolver.runStep();
	updateGridTexture();

	glBindTexture(GL_TEXTURE_2D, gridTexture);
	glBindVertexArray(gridVAO);
	glUseProgram(gridProgram);

	bool wireframe = false;

	if (wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDrawArrays(GL_TRIANGLES, 0, gridVBOSize);
	if (wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	debugTextureCounter++;
	if (debugTextureCounter >= gridSize.x * gridSize.y) {
		debugTextureCounter = 0;
	}
}

void FluidRenderer2D::updateGridQuad(const glm::ivec2 screenSize) {
	int totalGridWidth = screenSize.x - minimumMargins.x * 2;
	float tilePixelSize = totalGridWidth / (float) gridSize.x;
	if (tilePixelSize * gridSize.y > screenSize.y - minimumMargins.y * 2) {
		tilePixelSize = (screenSize.y - minimumMargins.y * 2) / (float) gridSize.y;
	}
	glm::vec2 quadPixelSize{
		tilePixelSize * gridSize.x,
		tilePixelSize * gridSize.y
	};

	glm::vec2 quadSize = 2.0f * quadPixelSize / glm::vec2(screenSize);

	const float quadVertexData[] = {
		quadSize.x / 2,  quadSize.y / 2, 0.0f, 1.0f, 1.0f,
		-quadSize.x / 2,  quadSize.y / 2, 0.0f, 0.0f, 1.0f,
		-quadSize.x / 2, -quadSize.y / 2, 0.0f, 0.0f, 0.0f,
		quadSize.x / 2,  quadSize.y / 2, 0.0f, 1.0f, 1.0f,
		-quadSize.x / 2, -quadSize.y / 2, 0.0f, 0.0f, 0.0f,
		quadSize.x / 2, -quadSize.y / 2, 0.0f, 1.0f, 0.0f
	};

	gridVBOSize = 6;

	glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertexData), quadVertexData, GL_STATIC_DRAW);
}

void FluidRenderer2D::updateGridTexture() {
	// Render grid rectangles
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gridTexture);

	switch (gridRenderMode) {
	case GridRenderMode::CellType:
		for (int x = 0; x < gridSize.x; x++) {
			for (int y = 0; y < gridSize.y; y++) {
				int i = (y * gridSize.x + x) * 4;

				switch (fluidSolver.getCellType(x, y)) {
				case FluidSim2dCPU::CELL_EMPTY:
					gridTextureData[i + 3] = 0;
					break;
				case FluidSim2dCPU::CELL_SOLID:
					gridTextureData[i + 0] = 130;
					gridTextureData[i + 1] = 130;
					gridTextureData[i + 2] = 130;
					gridTextureData[i + 3] = 255;
					break;
				case FluidSim2dCPU::CELL_FLUID:
					gridTextureData[i + 0] = 52;
					gridTextureData[i + 1] = 161;
					gridTextureData[i + 2] = 235;
					gridTextureData[i + 3] = 255;
					break;
				}
			}
		}

		break;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, gridSize.x, gridSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, gridTextureData.data());
	glBindTexture(GL_TEXTURE_2D, 0);
}
