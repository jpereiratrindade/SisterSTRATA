#define GLM_ENABLE_EXPERIMENTAL
#include "world3d/camera/Camera.hpp"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_inverse.hpp>

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

void Camera::orbit(float yawDelta, float pitchDelta) {
    float distance = 100.0f;
    // Heuristic pivot: Ray plane intersection or fixed
    if (front_.z < -0.1f) { 
        float t = -position_.z / front_.z;
        if (t > 0.0f) distance = t;
    }
    
    glm::vec3 pivot = position_ + front_ * distance;
    glm::vec3 relPos = position_ - pivot;

    // Use Quaternions for Arcball to allow full freedom (no clamps)
    // Rotate around World Up (Global Yaw)
    glm::quat qYaw = glm::angleAxis(glm::radians(-yawDelta), worldUp_);
    // Rotate around Camera Right (Local Pitch)
    glm::quat qPitch = glm::angleAxis(glm::radians(pitchDelta), right_);
    
    glm::quat qRot = qYaw * qPitch;

    // Update Position
    relPos = qRot * relPos;
    position_ = pivot + relPos;

    // Update Orientation
    front_ = glm::normalize(qRot * front_);
    right_ = glm::normalize(glm::cross(front_, worldUp_)); // Keep horizon stable? 
    // If we want FULL 3-axis (rolling), we shouldn't use worldUp_ for cross.
    // We should just rotate Up and Right.
    // But usually 'girar sobre eixos' implies stable horizon unless it's a space sim.
    // "sobre os três eixos" implies rolling is allowed/happens?
    // Let's stick to stable horizon first (Model Viewer style).
    // If they want 3 axes, maybe they mean X, Y, AND Z?
    // The previous implementation WAS stable horizon.
    // The "Limited" complaint might be the CLAMPING.
    
    // Let's allow full pitch rotation (over the pole).
    // To do that, we update Front and Up directly.
    up_ = glm::normalize(qRot * up_);
    right_ = glm::normalize(glm::cross(front_, up_));
    
    // Sync Euler for compatibility (best effort, though simple Euler fails at poles)
    pitch_ = glm::degrees(asin(front_.z));
    yaw_ = glm::degrees(atan2(front_.y, front_.x));
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

std::pair<glm::vec3, glm::vec3> Camera::getPickRay(glm::vec2 mousePos, glm::vec2 screenSize) const {
    // NDC: -1 to 1. 
    // Mouse (0,0) is top-left in SDL/ImGui.
    // NDC (-1,-1) is bottom-left in standard GL, but Vulkan has Y down.
    // However, our getProjectionMatrix() already multiplies by [1][-1], 
    // so we should treat it consistently.
    
    float x = (2.0f * mousePos.x) / screenSize.x - 1.0f;
    float y = (2.0f * mousePos.y) / screenSize.y - 1.0f; // Note: keeping it 0 to 1 -> -1 to 1
    
    glm::mat4 invProj = glm::inverse(getProjectionMatrix());
    glm::mat4 invView = glm::inverse(getViewMatrix());
    
    // Near plane ray
    glm::vec4 rayClip = glm::vec4(x, y, -1.0f, 1.0f);
    glm::vec4 rayEye = invProj * rayClip;
    rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);
    
    glm::vec3 rayWorld = glm::vec3(invView * rayEye);
    rayWorld = glm::normalize(rayWorld);
    
    return {position_, rayWorld};
}

} // namespace World3D
