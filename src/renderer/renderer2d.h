#pragma once
#include <glm/vec2.hpp>
#include <vector>

#include "../camera.h"
#include "../glCommon.h"
#include "../shader.h"

class FluidRenderer2D {
public:
	void initialize(const glm::ivec2 screenSize);
	void screenSizeUpdate(const glm::ivec2 newSize);
	void render(GLFWwindow* window, float aspectRatio, float elapsedSeconds);

private:
	// Margins in pixel size
	const glm::ivec2 minimumMargins{ 100, 50 };
	const glm::ivec2 gridSize{ 100, 100 };

	GLuint gridVAO;
	GLuint gridVBO;
	GLuint gridTexture;
	GLuint gridProgram;
	Shader gridFragmentShader;
	Shader gridVertexShader;
	int gridVertexBufferLength;

	int debugTextureCounter = 0;
	std::vector<unsigned char> debugImageData;

	void updateGridVertices(const glm::ivec2 screenSize);
};
