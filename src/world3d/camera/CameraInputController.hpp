#pragma once

#include "world3d/camera/Camera.hpp"
#include <SDL2/SDL.h>

namespace World3D {

class CameraInputController {
public:
    CameraInputController(Camera& camera);

    void update(float deltaTime);
    void processEvent(const SDL_Event& event);

private:
    Camera& camera_;
    
    // Input state
    bool moveForward_ = false;
    bool moveBackward_ = false;
    bool moveLeft_ = false;
    bool moveRight_ = false;
    bool moveUp_ = false;
    bool moveDown_ = false;
    bool rightMouseButtonDown_ = false;
    bool leftMouseButtonDown_ = false;

    float moveSpeed_ = 5.0f; // m/s
    float mouseSensitivity_ = 0.1f;
};

} // namespace World3D
