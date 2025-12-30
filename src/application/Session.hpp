#pragma once

#include "core/domain/Workspace.hpp"
#include "core/domain/fourth_dimension/Trajectory.hpp"
#include "application/ports/ILLMService.hpp"
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

    void setLLMService(std::unique_ptr<Ports::ILLMService> llmService) {
        llmService_ = std::move(llmService);
    }

    [[nodiscard]] Ports::ILLMService* getLLMService() const {
        return llmService_.get();
    }

private:
    std::unique_ptr<Core::Domain::Workspace> workspace_;
    Core::Domain::FourthDimension::Trajectory trajectory_;
    std::unique_ptr<Ports::ILLMService> llmService_;
};

} // namespace Application
