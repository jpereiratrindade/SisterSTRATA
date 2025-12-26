#pragma once

#include <SDL2/SDL.h>
#include <string>
#include <stdexcept>
#include <functional>

namespace Infrastructure::Windowing {

class Window {
public:
    Window(const std::string& title, int width, int height);
    ~Window();

    void pollEvents(const std::function<void(const SDL_Event&)>& onEvent);
    void swapBuffers();
    
    [[nodiscard]] bool shouldClose() const { return should_close_; }
    [[nodiscard]] SDL_Window* getNativeWindow() const { return window_; }
    [[nodiscard]] SDL_GLContext getGlContext() const { return gl_context_; }

private:
    SDL_Window* window_ = nullptr;
    SDL_GLContext gl_context_ = nullptr;
    bool should_close_ = false;
};

} // namespace Infrastructure::Windowing
