#pragma once
#include <glm/vec2.hpp>
#include <vector>

#include "../camera.h"
#include "../glCommon.h"
#include "../shader.h"
#include "../sim/sim2dCPU.h"

class FluidRenderer2D {
public:
	FluidRenderer2D();
	void initialize(const glm::ivec2 screenSize, std::string executableDirectory);
	void screenSizeUpdate(const glm::ivec2 newSize);
	void render(GLFWwindow* window, float aspectRatio, float elapsedSeconds);

private:
	enum class GridRenderMode {
		CellType
	};

	std::string executableDirectory;

	// Margins in pixel size
	const glm::ivec2 minimumMargins{ 100, 50 };
	const glm::ivec2 gridSize{ 4, 4 };

	FluidSim2dCPU::Solver fluidSolver{ gridSize };
	GLuint gridVAO;
	GLuint gridVBO;
	GLuint gridTexture;
	GLuint gridProgram;
	Shader gridFragmentShader;
	Shader gridVertexShader;
	int gridVBOSize;
	GridRenderMode gridRenderMode = GridRenderMode::CellType;
	std::vector<unsigned char> gridTextureData;

	int debugTextureCounter = 0;
	std::vector<unsigned char> debugImageData;

	void updateGridQuad(const glm::ivec2 screenSize);
	void updateGridTexture();
};
