cmake_minimum_required(VERSION 3.23)
project(FluidSim)

set(CMAKE_CXX_STANDARD 14)

# Set compiler flags based on architecture
if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
    SET(CMAKE_CXX_FLAGS "-O3")
else()
    SET(CMAKE_CXX_FLAGS "-O3 -mavx2 -mbmi2")
endif()

include(FetchContent)
FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG 43caca05c27aebaa8f8fc3064d47042dce6c7f8b
)
FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG e7ea71be039836da3a98cea55ae5569cb5eb885c
)

FetchContent_MakeAvailable(imgui)
FetchContent_MakeAvailable(glfw)

file(GLOB IMGUI
        ${imgui_SOURCE_DIR}/*.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
        ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp
        )

include_directories(${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends ${CMAKE_CURRENT_SOURCE_DIR})

find_package(OpenGL REQUIRED)

file(READ ${CMAKE_CURRENT_SOURCE_DIR}/particle.frag FRAG_STR)

file(READ ${CMAKE_CURRENT_SOURCE_DIR}/particle.vert VERT_STR)

set(SHADER_SOURCE_CPP "const char* P_FRAG_STR = R\"(${FRAG_STR})\";
const char* P_VERT_STR = R\"(${VERT_STR})\";")

file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/shaders.h "${SHADER_SOURCE_CPP}")

add_executable(
        FluidSim main.cpp Particle.cpp render.cpp simulation.cpp ParticleContainer.cpp HashContainer.cpp
        BinaryPartitionContainer.cpp ${IMGUI} gl.c
)
target_link_libraries(FluidSim PRIVATE glfw OpenGL::GL)