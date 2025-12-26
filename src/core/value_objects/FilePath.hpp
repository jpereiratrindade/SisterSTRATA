#pragma once

#include <filesystem>
#include <string>

namespace Core::ValueObjects {

class FilePath {
public:
    explicit FilePath(const std::string& path_str) : path_(path_str) {}
    explicit FilePath(const std::filesystem::path& path) : path_(path) {}

    [[nodiscard]] std::string toString() const { return path_.string(); }
    [[nodiscard]] std::filesystem::path toStdPath() const { return path_; }
    
    [[nodiscard]] bool exists() const {
        return std::filesystem::exists(path_);
    }

    [[nodiscard]] bool empty() const {
        return path_.empty();
    }

private:
    std::filesystem::path path_;
};

} // namespace Core::ValueObjects
