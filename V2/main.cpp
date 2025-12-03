#define SDL_MAIN_HANDLED
#include <SDL_2/SDL.h>
#include <glad/glad.h>
#include <iostream>
#include <direct.h>

#include "Shader/Shader.hpp"
#include "Window/Window.hpp"
#include "Mesh/Mesh.hpp"

#include "Renderer/VertexArray.hpp"
#include "Renderer/VertexLayout.hpp"
#include "Renderer/VertexBuffer.hpp"
#include "Renderer/Texture.hpp"

void getOpenGLversionDetails() {
    std::cout << "Vendor Version:           " << glGetString(GL_VENDOR) << "\n";
    std::cout << "Renderer Version:         " << glGetString(GL_RENDERER) << "\n";
    std::cout << "GL Version:               " << glGetString(GL_VERSION) << "\n";
    std::cout << "Shading Language Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n";
}

int main() {
    char buffer[512];
    _getcwd(buffer, 512);
    std::cout << "Working dir = " << buffer << "\n";

    Window window(800, 600, "Modular OpenGL");

    Shader shader(
        "Shaders/vertex_shader.glsl",
        "Shaders/fragment_shader.glsl"
    );

    shader.use();
    GLenum err = glGetError();
    std::cout << "shader error: " << err << "\n";
/*
    float model[] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0, 
        0.5f, 0.f, 0.f, 1
    };
*/

    float angle = 0.0f;


    float vertices[] = {
        //    x      y      r     g     b    u     v
             0.0f,  0.5f,   1.0f, 0.0f, 0.0f, 0.5f, 1.0f,
            -0.5f, -0.5f,   0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
             0.5f, -0.5f,   0.0f, 0.0f, 1.0f, 1.0f, 0.0f
    };

    VertexLayout layout;
    layout.push<float>(2); // position
    layout.push<float>(3); // color
    layout.push<float>(2); // uv

    Texture tex("Textures/bright-squares.png");


    shader.use();
    tex.bind(0); 
    shader.setInt("uTexture", 0);

    Mesh triangle(vertices, 21, layout);

    getOpenGLversionDetails();

    while (!window.shouldClose()) {
        window.pollEvents();
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        shader.use();

        angle += 0.01f;

        float c = cos(angle);
        float s = sin(angle);

        float model[16] = {
             c, s, 0, 0,
            -s, c, 0, 0,
             0, 0, 1, 0,
             0, 0, 0, 1
        };

        shader.setMat4("uModel", model);
     
        tex.bind(0);
        triangle.draw();

        window.swapBuffers();
    }
    return 0;
}