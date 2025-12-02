cmake_minimum_required(VERSION 3.10)
project(OpenGLProject)

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable compile_commands.json (useful for IDEs)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---- SDL2 ----
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# ---- Include Paths ----
# For GLAD headers
include_directories(${CMAKE_SOURCE_DIR}/include)

# For your own .hpp files (Shader.hpp, etc.)
include_directories(${CMAKE_SOURCE_DIR}/src)

# ---- Source Files ----
# Automatically collect ALL .cpp and .c files in src/
file(GLOB_RECURSE SOURCES
    ${CMAKE_SOURCE_DIR}/src/*.cpp
    ${CMAKE_SOURCE_DIR}/src/*.c
)

# ---- Executable ----
add_executable(RunOpenGL ${SOURCES})

# ---- Linking ----
target_link_libraries(RunOpenGL
    ${SDL2_LIBRARIES}
    GL         # OpenGL (Linux)
)
