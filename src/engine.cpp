#include <iostream>

#include "engine.h"
#include "glCommon.h"


Engine::Engine(GLFWwindow* window, const std::string& executableDirectory) : window(window), executableDirectory(executableDirectory) {
    previousFrameTime = glfwGetTime();
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    previousScreenSize = glm::ivec2(width, height);
    renderer2d.initialize(previousScreenSize, executableDirectory);
}

Engine::~Engine() {
    std::cout << "engine destructor" << std::endl;
}

void Engine::keyCallback(int key, int scanCode, int action, int mods) {

}

void Engine::mousePositionCallback(double x, double y) {

}

void Engine::mouseEnteredCallback(int entered) {

}

void Engine::renderFrame() {
    double frameTime = glfwGetTime();
    double elapsedSeconds = frameTime - previousFrameTime;
    previousFrameTime = frameTime;

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    float aspectRatio = width / (float)height;

    if (width != previousScreenSize.x || height != previousScreenSize.y) {
        renderer2d.screenSizeUpdate(glm::ivec2(width, height));
        previousScreenSize = glm::ivec2(width, height);
    }

    glfwPollEvents();

    ////ImGui_ImplOpenGL3_NewFrame();
    ////ImGui_ImplGlfw_NewFrame();
    ////ImGui::NewFrame();
    ////ImGui::ShowDemoWindow();

    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT);

    renderer2d.render(window, aspectRatio, (float) elapsedSeconds);

    //fluidRenderer.render(window, aspectRatio);

    ////ImGui::Render();
    ////ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
}

std::string Engine::getExecutableDirectory() {
    return executableDirectory;
}
