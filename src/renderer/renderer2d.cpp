#include <filesystem>
#include <iostream>
#include <vector>

#include "renderer2d.h"
#include "../glCommon.h"
#include "../camera.h"

extern std::string executableDirectory;

void FluidRenderer2D::initialize(const glm::ivec2 screenSize) {
	const int floatsPerQuad = 6 * (3 + 2);
	gridVertexBufferLength = static_cast<int>(gridSize.x * gridSize.y * floatsPerQuad);

	glGenVertexArrays(1, &gridVAO);
	glBindVertexArray(gridVAO);

	const int vertexPositionLocation = 0;
	const int vertexUVLocation = 1;

	glGenBuffers(1, &gridVBO);
	glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
	updateGridVertices(screenSize);
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
	gridFragmentShader.compileAndAttach(gridProgram, GL_FRAGMENT_SHADER, "noop_fragment.glsl");
	gridVertexShader.compileAndAttach(gridProgram, GL_VERTEX_SHADER, "noop_vertex.glsl");
	glLinkProgram(gridProgram);

	debugImageData.resize(gridSize.x * gridSize.y * 4);
}

void FluidRenderer2D::screenSizeUpdate(const glm::ivec2 newSize) {
	updateGridVertices(newSize);
}

void FluidRenderer2D::render(GLFWwindow* window, float aspectRatio, float elapsedSeconds) {
	// Render grid rectangles
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gridTexture);

	// Update for grid color data, slow but works for now
	for (int x = 0; x < gridSize.x; x++) {
		for (int y = 0; y < gridSize.y; y++) {
			int i = y * gridSize.x + x;
			if (i < debugTextureCounter) {
				debugImageData.at(4 * i) = 0;
				debugImageData.at(4 * i + 1) = 100;
				debugImageData.at(4 * i + 2) = 100;
			} else {
				debugImageData.at(4 * i) = 0;
				debugImageData.at(4 * i + 1) = 0;
				debugImageData.at(4 * i + 2) = 0;
			}
		}
	}
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, gridSize.x, gridSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, debugImageData.data());

	glBindVertexArray(gridVAO);
	glUseProgram(gridProgram);

	bool wireframe = false;

	if (wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDrawArrays(GL_TRIANGLES, 0, gridVertexBufferLength);
	if (wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	debugTextureCounter++;
	if (debugTextureCounter >= gridSize.x * gridSize.y) {
		debugTextureCounter = 0;
	}
}

void FluidRenderer2D::updateGridVertices(const glm::ivec2 screenSize) {
	int totalGridWidth = screenSize.x - minimumMargins.x * 2;
	float tilePixelSize = totalGridWidth / (float) gridSize.x;
	if (tilePixelSize * gridSize.y > screenSize.y - minimumMargins.y * 2) {
		tilePixelSize = (screenSize.y - minimumMargins.y * 2) / (float) gridSize.y;
	}

	// Size in [-1, 1] 2D screen space.
	const glm::vec2 tileSize{
		2 * tilePixelSize / (float) screenSize.x,
		2 * tilePixelSize / (float) screenSize.y
	};
	const glm::vec2 centerOffset{
		tileSize.x * (gridSize.x / 2 - 0.5f),
		tileSize.y * (gridSize.y / 2 - 0.5f),
	};

	const float squareVertices[] = {
		tileSize.x / 2,  tileSize.y / 2, 0.0f,
		-tileSize.x / 2,  tileSize.y / 2, 0.0f,
		-tileSize.x / 2, -tileSize.y / 2, 0.0f,
		tileSize.x / 2,  tileSize.y / 2, 0.0f,
		-tileSize.x / 2, -tileSize.y / 2, 0.0f,
		tileSize.x / 2, -tileSize.y / 2, 0.0f,
	};
	const int squareVerticesLength = sizeof(squareVertices) / sizeof(float);

	std::vector<float> gridVertices;
	gridVertices.reserve(gridSize.x * gridSize.y * squareVerticesLength);
	for (int x = 0; x < gridSize.x; x++) {
		for (int y = 0; y < gridSize.y; y++) {
			for (int i = 0; i < squareVerticesLength; i += 3) {
				gridVertices.push_back(squareVertices[i] + x * tileSize.x - centerOffset.x);
				gridVertices.push_back(squareVertices[i + 1] + y * tileSize.y - centerOffset.y);
				gridVertices.push_back(squareVertices[i + 2]);
				gridVertices.push_back(x / (float) gridSize.x);
				gridVertices.push_back(y / (float) gridSize.y);
			}
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
	glBufferData(GL_ARRAY_BUFFER, gridVertexBufferLength * sizeof(float), gridVertices.data(), GL_STATIC_DRAW);
}
