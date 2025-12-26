#include "infrastructure/window/Window.hpp"
#include <iostream>

namespace Infrastructure::Windowing {

Window::Window(const std::string& title, int width, int height) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
        throw std::runtime_error("Error: " + std::string(SDL_GetError()));
    }

    // GL 3.0 + GLSL 130
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    // Double buffer
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    window_ = SDL_CreateWindow(
        title.c_str(),
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width, height,
        SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE
    );

    if (!window_) {
        throw std::runtime_error("Error creating window: " + std::string(SDL_GetError()));
    }

    // GL Context not needed for Vulkan, but might be needed for ImGui OpenGL backend?
    // We are migrating the UI to Vulkan, so we don't need GL context anymore.
    // gl_context_ = SDL_GL_CreateContext(window_); 
    // SDL_GL_MakeCurrent(window_, gl_context_);
    // SDL_GL_SetSwapInterval(1); 
}

Window::~Window() {
    // if (gl_context_) SDL_GL_DeleteContext(gl_context_);
    if (window_) SDL_DestroyWindow(window_);
    SDL_Quit();
}

void Window::pollEvents(const std::function<void(const SDL_Event&)>& onEvent) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (onEvent) {
            onEvent(event);
        }
        if (event.type == SDL_QUIT) {
            should_close_ = true;
        }
        if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window_)) {
            should_close_ = true;
        }
    }
}

void Window::swapBuffers() {
    // SDL_GL_SwapWindow(window_); // Not used in Vulkan
}

} // namespace Infrastructure::Windowing
