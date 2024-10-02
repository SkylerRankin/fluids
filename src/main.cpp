#include <iostream>
#include <windows.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "engine.h"
#include "glCommon.h"
#include "renderer/renderer2d.h"

void errorCallback(int error, const char* message) {
    std::cout << "Error (" << error << "): " << message << std::endl;
}

void keyCallback(GLFWwindow* window, int key, int scanCode, int action, int mods) {
    Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
    engine->keyCallback(key, scanCode, action, mods);
}

void mousePositionCallback(GLFWwindow* window, double x, double y) {
    Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
    engine->mousePositionCallback(x, y);
}

void mouseEnteredCallback(GLFWwindow* window, int entered) {
    Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
    engine->mouseEnteredCallback(entered);
}

std::string getExecutableDirectory() {
    WCHAR wideExecutableDirectory[MAX_PATH];
    GetModuleFileNameW(NULL, wideExecutableDirectory, MAX_PATH);
    unsigned int endIndex = MAX_PATH - 1;
    while (wideExecutableDirectory[endIndex] != '\\') {
        wideExecutableDirectory[endIndex--] = 0;
    }

    std::string executableDirectory;
    std::wstring wideString{ wideExecutableDirectory, endIndex };
    executableDirectory.resize(wideString.length());
    wcstombs_s(nullptr, executableDirectory.data(), wideString.length() + 1, wideString.data(), wideString.size());
    return executableDirectory;
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to init GLFW." << std::endl;
        glfwTerminate();
        return EXIT_FAILURE;
    }

    GLFWwindow* window = glfwCreateWindow(1000, 500, "fluid-sim", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window." << std::endl;
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwSetErrorCallback(errorCallback);
    glfwSetKeyCallback(window, &keyCallback);
    glfwSetCursorPosCallback(window, &mousePositionCallback);
    glfwSetCursorEnterCallback(window, &mouseEnteredCallback);
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwMakeContextCurrent(window);
    // Removes vsync
    //glfwSwapInterval(0);
    gladLoadGL();

    const GLubyte* version = glGetString(GL_VERSION);
    std::cout << "OpenGL version: " << version << std::endl;

    std::string executableDirectory = getExecutableDirectory();
    std::cout << "Executable directory: " << executableDirectory << std::endl;

    Engine engine{window, executableDirectory };
    glfwSetWindowUserPointer(window, &engine);

    while (!glfwWindowShouldClose(window)) {
        engine.renderFrame();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
}
