#pragma once

#include "core/domain/Workspace.hpp"
#include "core/domain/fourth_dimension/Trajectory.hpp"
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
        trajectory_.clear();
    }

    [[nodiscard]] Core::Domain::FourthDimension::Trajectory& getTrajectory() {
        return trajectory_;
    }

private:
    std::unique_ptr<Core::Domain::Workspace> workspace_;
    Core::Domain::FourthDimension::Trajectory trajectory_;
};

} // namespace Application
