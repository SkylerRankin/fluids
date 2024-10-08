cmake_minimum_required(VERSION 3.10)
set(TARGET fluids)
project(${TARGET} VERSION 0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)

# Prevents the ZERO_CHECK project from being generated
set(CMAKE_SUPPRESS_REGENERATION true)

# GLAD local source files
include_directories(lib/glad/include)
set(GLAD_SOURCES lib/glad/src/glad.c)

# Fetch external libraries: glfw-3.4, glm-1.0.1, imgui-1.91.1
include(FetchContent)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY "https://github.com/glfw/glfw.git"
    GIT_TAG 3.4
)
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(
    glm
    GIT_REPOSITORY "https://github.com/g-truc/glm.git"
    GIT_TAG 1.0.1
)
FetchContent_MakeAvailable(glm)

FetchContent_Declare(
    imgui_external
    GIT_REPOSITORY "https://github.com/ocornut/imgui.git"
    GIT_TAG v1.91.1
)
FetchContent_MakeAvailable(imgui_external)
add_library(imgui
	${imgui_external_SOURCE_DIR}/imgui.cpp
    ${imgui_external_SOURCE_DIR}/imgui_demo.cpp
	${imgui_external_SOURCE_DIR}/imgui_draw.cpp
	${imgui_external_SOURCE_DIR}/imgui_tables.cpp
	${imgui_external_SOURCE_DIR}/imgui_widgets.cpp
	${imgui_external_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_external_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)
include_directories(${imgui_external_SOURCE_DIR}/backends)
target_include_directories(imgui PUBLIC ${imgui_external_SOURCE_DIR})

# List source files
set(CXX_SOURCES
    src/fluids.cpp
    ${GLAD_SOURCES}
)

# List header files
set(CXX_HEADERS)
set_source_files_properties(${CXX_HEADERS} PROPERTIES HEADER_FILE_ONLY true)

# Create executable and link used libraries.
add_executable(${TARGET} ${CXX_SOURCES} ${CXX_HEADERS} ${GLAD_SOURCES})
target_link_libraries(imgui PRIVATE glfw)
target_link_libraries(fluids PRIVATE glfw glm imgui)

# Controls if a command prompt window is opened when running the executable on Windows. Set to true to hide the window.
set(HIDE_COMMAND_WINDOW false)
if (${WIN32} AND ${HIDE_COMMAND_WINDOW})
    set_target_properties(${TARGET} PROPERTIES LINK_FLAGS "/ENTRY:mainCRTStartup /SUBSYSTEM:WINDOWS")
endif()


