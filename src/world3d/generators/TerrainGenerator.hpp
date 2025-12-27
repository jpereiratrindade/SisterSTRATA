#pragma once

#include <string>

namespace World3D::Generators {

class TerrainGenerator {
public:
    enum class Type {
        Hills = 0,
        Mountains = 1,
        Flat = 2,
        Canyon = 3
    };

    static bool generate(const std::string& filename, int width, int height, float spacing, Type type);

private:
    static float getHeight(float x, float y, float cx, float cy, Type type);
};

} // namespace World3D::Generators
