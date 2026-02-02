#pragma once
#include <vector>
#include <cmath>
#include <algorithm>
#include <glm/glm.hpp>

namespace Core::Domain::Shared {

/**
 * @brief Metadata describing a detected grid structure in a 1D vertex array.
 */
struct GridInfo {
    int width = 0;       ///< Number of columns (outer loop)
    int height = 0;      ///< Number of rows (inner loop)
    float spacingX = 1.0f; ///< Distance between columns
    float spacingY = 1.0f; ///< Distance between rows
    bool isValid = false;  ///< True if a valid grid was identified
};

/**
 * @brief Utility for terrain-specific geometric calculations.
 */
class SlopeHelper {
public:
    /**
     * @brief Attempts to identify if a vertex buffer follows a regular grid pattern.
     * @param vertices List of vertices to analyze.
     * @return GridInfo with detected dimensions and spacing.
     */
    template<typename TVertex>
    static GridInfo detectGrid(const std::vector<TVertex>& vertices) {
        GridInfo info;
        size_t count = vertices.size();
        if (count < 4) return info;

        // Try to identify the height of the grid (inner loop size)
        // We look for the first index where Y decreases significantly, indicating a wrap-around.
        const float eps = 1e-4f;
        for (size_t i = 1; i < count; ++i) {
            if (vertices[i].pos.y < vertices[i - 1].pos.y - eps) {
                info.height = static_cast<int>(i);
                break;
            }
        }

        // If height was found and count is divisible, we have a candidate width.
        if (info.height > 0 && (count % static_cast<size_t>(info.height) == 0)) {
            info.width = static_cast<int>(count / static_cast<size_t>(info.height));
        } else {
            // Fallback: Check if it's a perfect square (common for LIDAR/Grids)
            int gridSide = static_cast<int>(std::sqrt(static_cast<double>(count)));
            if (gridSide * gridSide == static_cast<int>(count)) {
                info.width = gridSide;
                info.height = gridSide;
            }
        }

        // Validate dimensions and calculate spacing
        if (info.width >= 2 && info.height >= 2 && (static_cast<size_t>(info.width) * static_cast<size_t>(info.height) == count)) {
            // spacingY (inner loop difference)
            const auto& v0 = vertices[0].pos;
            const auto& v1 = vertices[1].pos;
            info.spacingY = std::sqrt(std::pow(v1.x - v0.x, 2) + std::pow(v1.y - v0.y, 2));

            // spacingX (outer loop difference)
            const auto& vCol1 = vertices[info.height].pos;
            info.spacingX = std::sqrt(std::pow(vCol1.x - v0.x, 2) + std::pow(vCol1.y - v0.y, 2));
            
            if (info.spacingX > 1e-4f && info.spacingY > 1e-4f) {
                info.isValid = true;
            }
        }

        return info;
    }

    /**
     * @brief Calculates the slope in degrees at a specific grid coordinate using central differences.
     * @param vertices The vertex buffer.
     * @param info Grid metadata (must be valid).
     * @param i Column index.
     * @param j Row index.
     * @return Slope in degrees [0, 90].
     */
    template<typename TVertex>
    static float calculateSlopeDeg(const std::vector<TVertex>& vertices, const GridInfo& info, int i, int j) {
        if (!info.isValid) return 0.0f;

        auto getZ = [&](int x, int y) -> float {
            x = std::clamp(x, 0, info.width - 1);
            y = std::clamp(y, 0, info.height - 1);
            size_t idx = static_cast<size_t>(x) * static_cast<size_t>(info.height) + static_cast<size_t>(y);
            return static_cast<float>(vertices[idx].pos.z);
        };

        // Central difference approximation
        float dzdx = (getZ(i + 1, j) - getZ(i - 1, j)) / (2.0f * info.spacingX);
        float dzdy = (getZ(i, j + 1) - getZ(i, j - 1)) / (2.0f * info.spacingY);
        
        float slopeMagnitude = std::sqrt(dzdx * dzdx + dzdy * dzdy);
        return glm::degrees(std::atan(slopeMagnitude));
    }
    
    /**
     * @brief Checks if the normal vectors in the buffer are effectively uniform (pointing straight up).
     */
    template<typename TVertex>
    static bool areNormalsUniformUp(const std::vector<TVertex>& vertices, float threshold = 0.999f) {
        if (vertices.empty()) return true;
        // Sample a few points to see if they all point UP
        size_t sampleCount = 50;
        size_t step = std::max<size_t>(1, vertices.size() / sampleCount);
        for (size_t i = 0; i < vertices.size(); i += step) {
            if (vertices[i].normal.z < threshold) return false;
        }
        return true;
    }
};

} // namespace Core::Domain::Shared
