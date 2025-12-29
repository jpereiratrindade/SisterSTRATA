#pragma once

#include <string>
#include <functional>

namespace World3D::Generators {

class TerrainGenerator {
public:
    enum class Type {
        Flat = 0,
        Hills = 1,
        Mountains = 2,
        Canyon = 3,
        Showcase = 4
    };

    static bool generate(const std::string& filename, int width, int height, float spacing, Type type, std::function<void(float, const std::string&)> onProgress = nullptr);

private:
    static float getHeight(float x, float y, float cx, float cy, Type type);
};

} // namespace World3D::Generators
