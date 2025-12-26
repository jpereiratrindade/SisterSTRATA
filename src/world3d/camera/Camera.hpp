#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace World3D {

class Camera {
public:
    Camera(glm::vec3 position, float fov, float aspectRatio);

    void move(glm::vec3 delta);
    void rotate(float yawDelta, float pitchDelta);
    void setAspectRatio(float aspectRatio);

    [[nodiscard]] glm::mat4 getViewMatrix() const;
    [[nodiscard]] glm::mat4 getProjectionMatrix() const;
    [[nodiscard]] glm::vec3 getPosition() const { return position_; }

private:
    void updateVectors();

    glm::vec3 position_;
    glm::vec3 front_;
    glm::vec3 up_;
    glm::vec3 right_;
    glm::vec3 worldUp_;

    float yaw_;
    float pitch_;
    float fov_;
    float aspectRatio_;
    float nearPlane_;
    float farPlane_;
};

} // namespace World3D
