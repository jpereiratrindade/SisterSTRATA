#include "world3d/camera/Camera.hpp"

namespace World3D {

Camera::Camera(glm::vec3 position, float fov, float aspectRatio)
    : position_(position),
      front_(glm::vec3(0.0f, 0.0f, -1.0f)),
      worldUp_(glm::vec3(0.0f, 0.0f, 1.0f)), // Z-up for scientific/terrain usually
      yaw_(90.0f),
      pitch_(0.0f),
      fov_(fov),
      aspectRatio_(aspectRatio),
      nearPlane_(0.1f),
      farPlane_(10000.0f) { // Large far plane for scientific data
    
    updateVectors();
}

void Camera::move(glm::vec3 delta) {
    position_ += front_ * delta.x;
    position_ += right_ * delta.y;
    position_ += worldUp_ * delta.z; // Move along global Z for up/down
}

void Camera::rotate(float yawDelta, float pitchDelta) {
    yaw_ += yawDelta;
    pitch_ += pitchDelta;

    if (pitch_ > 89.0f) pitch_ = 89.0f;
    if (pitch_ < -89.0f) pitch_ = -89.0f;

    updateVectors();
}

void Camera::setAspectRatio(float aspectRatio) {
    aspectRatio_ = aspectRatio;
}

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(position_, position_ + front_, up_);
}

glm::mat4 Camera::getProjectionMatrix() const {
    auto proj = glm::perspective(glm::radians(fov_), aspectRatio_, nearPlane_, farPlane_);
    proj[1][1] *= -1; // Vulkan flip Y
    return proj;
}

void Camera::updateVectors() {
    glm::vec3 newFront;
    // Standard math for Z-up specific systems might vary, but assuming standard GL conventions rotated:
    // If Z is up:
    // Yaw should be around Z. Pitch around Right.
    // Let's stick to standard Y-up math and just set worldUp to (0,0,1) and see if it behaves as expected or if we need conversion.
    // Standard GL: Y is up, -Z is forward.
    // Our scientific: Z is up.
    
    // Let's implement calculating front for Z-up:
    newFront.x = cos(glm::radians(yaw_)) * cos(glm::radians(pitch_));
    newFront.y = sin(glm::radians(yaw_)) * cos(glm::radians(pitch_));
    newFront.z = sin(glm::radians(pitch_));
    
    front_ = glm::normalize(newFront);
    right_ = glm::normalize(glm::cross(front_, worldUp_));
    up_    = glm::normalize(glm::cross(right_, front_));
}

} // namespace World3D
