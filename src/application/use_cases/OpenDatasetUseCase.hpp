#pragma once

#include "application/Session.hpp"
#include "application/ports/IFileSystem.hpp"
#include <string>
#include <stdexcept>

namespace Application::UseCases {

struct OpenDatasetRequest {
    std::string name;
    std::string path;
};

class OpenDatasetUseCase {
public:
    explicit OpenDatasetUseCase(Session& session, const Ports::IFileSystem& fileSystem)
        : session_(session), fileSystem_(fileSystem) {}

    void execute(const OpenDatasetRequest& request) {
        if (!fileSystem_.exists(request.path)) {
            // In a real app we might use a Result<T, E> type or specific exception
            throw std::runtime_error("File does not exist: " + request.path);
        }

        std::string fullPath = fileSystem_.getAbsolutePath(request.path);
        session_.getWorkspace().addDataset(request.name, fullPath);
    }

private:
    Session& session_;
    const Ports::IFileSystem& fileSystem_;
};

} // namespace Application::UseCases
