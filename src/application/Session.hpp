#pragma once

#include "core/domain/Workspace.hpp"
#include <memory>

namespace Application {

class Session {
public:
    Session() : workspace_(std::make_unique<Core::Domain::Workspace>()) {}

    [[nodiscard]] Core::Domain::Workspace& getWorkspace() const {
        return *workspace_;
    }

    // Ability to reset session if needed
    void newSession() {
        workspace_ = std::make_unique<Core::Domain::Workspace>();
    }

private:
    std::unique_ptr<Core::Domain::Workspace> workspace_;
};

} // namespace Application
