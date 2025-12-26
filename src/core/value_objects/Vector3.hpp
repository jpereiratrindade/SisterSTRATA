#pragma once

#include <cmath>

namespace Core::ValueObjects {

struct Vector3 {
    double x;
    double y;
    double z;

    constexpr Vector3(double x_val = 0.0, double y_val = 0.0, double z_val = 0.0)
        : x(x_val), y(y_val), z(z_val) {}

    constexpr Vector3 operator+(const Vector3& other) const {
        return {x + other.x, y + other.y, z + other.z};
    }

    constexpr Vector3 operator-(const Vector3& other) const {
        return {x - other.x, y - other.y, z - other.z};
    }

    constexpr double magnitudeSquared() const {
        return x * x + y * y + z * z;
    }

    double magnitude() const {
        return std::sqrt(magnitudeSquared());
    }
};

} // namespace Core::ValueObjects
