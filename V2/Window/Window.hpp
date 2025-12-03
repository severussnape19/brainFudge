#pragma once

#include <SDL_2/SDL.h>
#include <string>

class Window {
public:
    Window(int width, int height, const std::string& title);
    ~Window();

    bool shouldClose() const;
    void pollEvents();
    void swapBuffers() const;

    SDL_Window* getSDLWindow() const { return window; }
private:
    SDL_Window *window = nullptr;
    SDL_GLContext glContext = nullptr;
    bool closeRequested = false;
};