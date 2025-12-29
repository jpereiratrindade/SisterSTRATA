#pragma once

#include <cstdint>
#include <vector>

namespace Core::Domain::Hydro {

/**
 * @brief Direction of flow in the D8 algorithm.
 */
enum class FlowDir : int8_t {
    East = 0,
    SouthEast = 1,
    South = 2,
    SouthWest = 3,
    West = 4,
    NorthWest = 5,
    North = 6,
    NorthEast = 7,
    Sink = -1
};

/**
 * @brief Strategy for handling sinks (local depressions).
 */
enum class SinkHandling {
    Ignore,
    FillDepressions, // Future implementation
    Breach           // Future implementation
};

/**
 * @brief Pure domain value object representing terrain elevation.
 * Decouples the domain from the 3D engine's vertex format.
 */
struct ElevationGrid {
    int width;
    int height;
    std::vector<float> z; // Elevation values

    float get(int x, int y) const {
        if (x < 0 || x >= width || y < 0 || y >= height) return -9999.0f; // Boundary
        return z[y * width + x];
    }
};

} // namespace Core::Domain::Hydro
