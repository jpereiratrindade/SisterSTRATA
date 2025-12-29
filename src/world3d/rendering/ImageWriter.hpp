#pragma once

#include <string>
#include <vector>

namespace World3D::Rendering {

bool WritePng(const std::string& path, const std::vector<unsigned char>& rgba, int width, int height);

} // namespace World3D::Rendering
