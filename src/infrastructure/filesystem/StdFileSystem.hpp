#pragma once

#include "application/ports/IFileSystem.hpp"
#include <filesystem>

namespace Infrastructure::FileSystem {

class StdFileSystem : public Application::Ports::IFileSystem {
public:
    [[nodiscard]] bool exists(const std::string& path) const override {
        return std::filesystem::exists(path);
    }

    [[nodiscard]] std::string getAbsolutePath(const std::string& path) const override {
        return std::filesystem::absolute(path).string();
    }
};

} // namespace Infrastructure::FileSystem
