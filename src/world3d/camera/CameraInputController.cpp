#include "world3d/camera/CameraInputController.hpp"

namespace World3D {

CameraInputController::CameraInputController(Camera& camera)
    : camera_(camera) {}

void CameraInputController::update(float deltaTime) {
    glm::vec3 moveDir(0.0f);

    if (moveForward_) moveDir.x += 1.0f;
    if (moveBackward_) moveDir.x -= 1.0f;
    if (moveRight_) moveDir.y += 1.0f;
    if (moveLeft_) moveDir.y -= 1.0f;
    if (moveUp_) moveDir.z += 1.0f;
    if (moveDown_) moveDir.z -= 1.0f;

    if (glm::length(moveDir) > 0.0f) {
        moveDir = glm::normalize(moveDir);
        camera_.move(moveDir * moveSpeed_ * deltaTime);
    }
}

void CameraInputController::processEvent(const SDL_Event& event) {
    if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
            case SDLK_w: moveForward_ = true; break;
            case SDLK_s: moveBackward_ = true; break;
            case SDLK_a: moveLeft_ = true; break;
            case SDLK_d: moveRight_ = true; break;
            case SDLK_e: moveUp_ = true; break;
            case SDLK_q: moveDown_ = true; break;
            case SDLK_LSHIFT: moveSpeed_ = 20.0f; break; // Sprint
        }
    } else if (event.type == SDL_KEYUP) {
        switch (event.key.keysym.sym) {
            case SDLK_w: moveForward_ = false; break;
            case SDLK_s: moveBackward_ = false; break;
            case SDLK_a: moveLeft_ = false; break;
            case SDLK_d: moveRight_ = false; break;
            case SDLK_e: moveUp_ = false; break;
            case SDLK_q: moveDown_ = false; break;
            case SDLK_LSHIFT: moveSpeed_ = 5.0f; break;
        }
    } else if (event.type == SDL_MOUSEBUTTONDOWN) {
        if (event.button.button == SDL_BUTTON_RIGHT) {
            rightMouseButtonDown_ = true;
            SDL_SetRelativeMouseMode(SDL_TRUE);
        } else if (event.button.button == SDL_BUTTON_LEFT) {
            leftMouseButtonDown_ = true;
            SDL_SetRelativeMouseMode(SDL_TRUE);
        }
    } else if (event.type == SDL_MOUSEBUTTONUP) {
        if (event.button.button == SDL_BUTTON_RIGHT) {
            rightMouseButtonDown_ = false;
            SDL_SetRelativeMouseMode(SDL_FALSE);
        } else if (event.button.button == SDL_BUTTON_LEFT) {
            leftMouseButtonDown_ = false;
            SDL_SetRelativeMouseMode(SDL_FALSE);
        }
    } else if (event.type == SDL_MOUSEMOTION) {
        if (rightMouseButtonDown_ || leftMouseButtonDown_) {
            camera_.rotate(
                event.motion.xrel * mouseSensitivity_,
                -event.motion.yrel * mouseSensitivity_ // Invert Y typically
            );
        }
    }
}

} // namespace World3D
