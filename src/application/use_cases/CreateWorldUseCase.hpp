#pragma once

#include "application/Session.hpp"
#include <string>

namespace Application::UseCases {

struct CreateWorldRequest {
    std::string name;
    uint32_t width;
    uint32_t height;
};

class CreateWorldUseCase {
public:
    explicit CreateWorldUseCase(Session& session) : session_(session) {}

    void execute(const CreateWorldRequest& request) {
        session_.getWorkspace().createWorld(request.name, request.width, request.height);
    }

private:
    Session& session_;
};

} // namespace Application::UseCases
