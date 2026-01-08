#pragma once

#include "core/domain/Workspace.hpp"
#include "core/domain/fourth_dimension/Trajectory.hpp"
#include "application/ports/ILLMService.hpp"
#include "src/observational/narrative/aggregates/NarrativeObservationSystem.hpp"
#include <memory>

namespace Application {

class Session {
public:
    Session() 
        : workspace_(std::make_unique<Core::Domain::Workspace>()),
          narrativeSystem_(std::make_unique<SisterSTRATA::Observational::Narrative::NarrativeObservationSystem>()) 
    {}

    [[nodiscard]] Core::Domain::Workspace& getWorkspace() const {
        return *workspace_;
    }

    // Ability to reset session if needed
    void newSession() {
        workspace_ = std::make_unique<Core::Domain::Workspace>();
        narrativeSystem_ = std::make_unique<SisterSTRATA::Observational::Narrative::NarrativeObservationSystem>();
        trajectory_.clear();
    }

    [[nodiscard]] Core::Domain::FourthDimension::Trajectory& getTrajectory() {
        return trajectory_;
    }

    [[nodiscard]] SisterSTRATA::Observational::Narrative::NarrativeObservationSystem& getNarrativeSystem() const {
        return *narrativeSystem_;
    }

    void setLLMService(std::unique_ptr<Ports::ILLMService> llmService) {
        llmService_ = std::move(llmService);
    }

    [[nodiscard]] Ports::ILLMService* getLLMService() const {
        return llmService_.get();
    }

private:
    std::unique_ptr<Core::Domain::Workspace> workspace_;
    std::unique_ptr<SisterSTRATA::Observational::Narrative::NarrativeObservationSystem> narrativeSystem_;
    Core::Domain::FourthDimension::Trajectory trajectory_;
    std::unique_ptr<Ports::ILLMService> llmService_;
};

} // namespace Application
