#include "Window.hpp"
#include <glad/glad.h>
#include <iostream>

Window::Window(int width, int height, const std::string& title) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_INIT error: " << SDL_GetError() << "\n";
        exit(1);
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    window = SDL_CreateWindow(
        title.c_str(),
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width, height,
        SDL_WINDOW_OPENGL
    );

    if (!window) {
        std::cerr << "Window creation failed: " << SDL_GetError() << "\n";
        exit(1);
    }

    glContext = SDL_GL_CreateContext(window);
    if (!glContext) {
        std::cerr << "OpenGL context creation failed: " << SDL_GetError() << "\n";
        exit(1);
    }

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        std::cerr << "Failed to initialize GLAD!\n";
        exit(1);
    }
}

void Window::pollEvents() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            closeRequested = true;
        }
    }
}

void Window::swapBuffers() const {
    SDL_GL_SwapWindow(window);
}

bool Window::shouldClose() const {
    return closeRequested;
}

Window::~Window() {
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
}