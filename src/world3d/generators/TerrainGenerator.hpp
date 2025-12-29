#pragma once

#include <string>
#include <functional>

namespace World3D::Generators {

class TerrainGenerator {
public:
    /**
     * @brief Defines the available Procedural Generation patterns.
     */
    enum class Type {
        Flat = 0,      ///< Low amplitude noise, mostly level terrain.
        Hills = 1,     ///< Rolling hills with moderate frequency.
        Mountains = 2, ///< High amplitude, rugged terrain with steep peaks.
        Canyon = 3,    ///< Erosion-like patterns with deep valleys.
        Showcase = 4   ///< Optimized demo terrain with Plateau (Latossolo), Peaks (Neossolo), and Valleys (Gleissolo).
    };

    /**
     * @brief Generates a heightmap mesh and saves it to an OBJ file.
     * 
     * @param filename Output path for the .obj file.
     * @param width Width of the grid (vertices).
     * @param height Height of the grid (vertices).
     * @param spacing Distance between vertices in world units.
     * @param type The terrain generation pattern to use.
     * @param onProgress Optional callback for progress reporting (0.0 to 1.0).
     * @return true if generation and file writing were successful.
     */
    static bool generate(const std::string& filename, int width, int height, float spacing, Type type, std::function<void(float, const std::string&)> onProgress = nullptr);

private:
    static float getHeight(float x, float y, float cx, float cy, Type type);
};

} // namespace World3D::Generators
