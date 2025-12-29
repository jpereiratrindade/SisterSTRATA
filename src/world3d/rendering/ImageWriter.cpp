#include "world3d/rendering/ImageWriter.hpp"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

namespace World3D::Rendering {

bool WritePng(const std::string& path, const std::vector<unsigned char>& rgba, int width, int height) {
    if (width <= 0 || height <= 0) return false;
    const int stride = width * 4;
    return stbi_write_png(path.c_str(), width, height, 4, rgba.data(), stride) != 0;
}

} // namespace World3D::Rendering
