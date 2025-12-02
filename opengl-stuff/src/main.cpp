#include <SDL2/SDL.h>
#include <glad/glad.h>
#include <iostream>

#include "Shader/Shader.hpp"
#include "Window/Window.hpp"
#include "Mesh/Mesh.hpp"

#include "Renderer/VertexArray.hpp"
#include "Renderer/VertexLayout.hpp"
#include "Renderer/VertexBuffer.hpp"

void getOpenGLversionDetails() {
    std::cout << "Vendor Version:           " << glGetString(GL_VENDOR) << "\n";
    std::cout << "Renderer Version:         " << glGetString(GL_RENDERER) << "\n";
    std::cout << "GL Version:               " << glGetString(GL_VERSION) << "\n";
    std::cout << "Shading Language Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n";
}

int main() {

    Window window(800, 600, "Modular OpenGL");

    Shader shader(
        "../src/Shaders/vertex_shader.glsl", 
        "../src/Shaders/fragment_shader.glsl"
    );

    float vertices[] = {
//       x       y      r      g      b
         0.0f,  0.5f,   1.0f, 0.0f, 0.0f, // red top
        -0.5f, -0.5f,   0.0f, 1.0f, 0.0f, // green left
         0.5f, -0.5f,   0.0f, 0.0f, 1.0f // blue right
    };

    VertexLayout layout;
    layout.push<float>(2); // position
    layout.push<float>(3); // color

    Mesh triangle(vertices, 15, layout);

    getOpenGLversionDetails();

    while (!window.shouldClose()) {
        window.pollEvents();
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        shader.use();
        triangle.draw();
        
        window.swapBuffers();
    }
    return 0;
}