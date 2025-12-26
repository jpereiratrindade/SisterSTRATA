#pragma once

#include <string>
#include <vector>

namespace Application::Ports {

class IFileSystem {
public:
    virtual ~IFileSystem() = default;

    [[nodiscard]] virtual bool exists(const std::string& path) const = 0;
    [[nodiscard]] virtual std::string getAbsolutePath(const std::string& path) const = 0;
};

} // namespace Application::Ports
