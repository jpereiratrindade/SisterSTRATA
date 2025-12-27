#pragma once

#include "world3d/camera/Camera.hpp"
#include <SDL2/SDL.h>

namespace World3D {

class CameraInputController {
public:
    CameraInputController(Camera& camera);

    void update(float deltaTime);
    void processEvent(const SDL_Event& event);
    
    void setMoveSpeed(float speed) { baseSpeed_ = speed; }

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

    bool isSprinting_ = false;

    float baseSpeed_ = 2.5f; // m/s
    float sprintMultiplier_ = 4.0f; // 4x speed when sprinting
    float mouseSensitivity_ = 0.1f;
};

} // namespace World3D
