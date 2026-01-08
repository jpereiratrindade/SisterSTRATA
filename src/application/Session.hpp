#pragma once

#include "core/domain/Workspace.hpp"
#include "core/domain/fourth_dimension/Trajectory.hpp"
#include "application/ports/ILLMService.hpp"
#include "application/dtos/DiscursiveSystemDTO.hpp"
#include "application/dtos/RecommendationSnapshotDTO.hpp"
#include "application/dtos/RecommendationTrajectoryDTO.hpp"
#include "application/dtos/NarrativeDTOs.hpp"
#include "application/mappers/ObservationalMappers.hpp"
#include "src/observational/narrative/aggregates/NarrativeObservationSystem.hpp"
#include "src/observational/discursive/aggregates/DiscursiveSystemRepository.hpp"
#include "src/observational/recommendation/aggregates/RecommendationTrajectory.hpp"
#include <memory>
#include <vector>

namespace Application {

class Session {
public:
    Session() 
        : workspace_(std::make_unique<Core::Domain::Workspace>()),
          narrativeSystem_(std::make_unique<SisterSTRATA::Observational::Narrative::NarrativeObservationSystem>()),
          discursiveSystemRepository_(std::make_unique<SisterSTRATA::Observational::Discursive::DiscursiveSystemRepository>())
    {}

    [[nodiscard]] Core::Domain::Workspace& getWorkspace() const {
        return *workspace_;
    }

    // Ability to reset session if needed
    void newSession() {
        workspace_ = std::make_unique<Core::Domain::Workspace>();
        narrativeSystem_ = std::make_unique<SisterSTRATA::Observational::Narrative::NarrativeObservationSystem>();
        discursiveSystemRepository_ = std::make_unique<SisterSTRATA::Observational::Discursive::DiscursiveSystemRepository>();
        recommendationTrajectory_ = SisterSTRATA::Observational::Recommendation::RecommendationTrajectory();
        trajectory_.clear();
    }

    [[nodiscard]] Core::Domain::FourthDimension::Trajectory& getTrajectory() {
        return trajectory_;
    }

    [[nodiscard]] SisterSTRATA::Observational::Narrative::NarrativeObservationSystem& getNarrativeSystem() const {
        return *narrativeSystem_;
    }

    [[nodiscard]] std::vector<Application::DTO::NarrativeStateDTO> getNarrativeStateDTOs() const {
        std::vector<Application::DTO::NarrativeStateDTO> dtos;
        for (const auto& state : narrativeSystem_->getHistory()) {
            dtos.push_back(Application::Mappers::Narrative::toDTO(state));
        }
        return dtos;
    }

    void registerNarrativeStateDTO(const Application::DTO::NarrativeStateDTO& dto) {
        narrativeSystem_->registerObservation(Application::Mappers::Narrative::toDomain(dto));
    }

    void loadNarrativeFromFile(const std::string& path) {
        narrativeSystem_->deserialize(path);
    }

    void saveNarrativeToFile(const std::string& path) const {
        narrativeSystem_->serialize(path);
    }

    [[nodiscard]] SisterSTRATA::Observational::Discursive::DiscursiveSystemRepository& getDiscursiveSystemRepository() const {
        return *discursiveSystemRepository_;
    }

    [[nodiscard]] SisterSTRATA::Observational::Recommendation::RecommendationTrajectory& getRecommendationTrajectory() {
        return recommendationTrajectory_;
    }

    [[nodiscard]] std::vector<Application::DTO::DiscursiveSystemDTO> getDiscursiveSystemDTOs() const {
        std::vector<Application::DTO::DiscursiveSystemDTO> dtos;
        for (const auto& system : discursiveSystemRepository_->getSystems()) {
            dtos.push_back(Application::Mappers::Discursive::toDTO(system));
        }
        return dtos;
    }

    [[nodiscard]] Application::DTO::RecommendationTrajectoryDTO getRecommendationTrajectoryDTO() const {
        return Application::Mappers::Recommendation::toDTO(recommendationTrajectory_);
    }

    void registerDiscursiveSystemDTO(const Application::DTO::DiscursiveSystemDTO& dto) {
        discursiveSystemRepository_->registerSystem(Application::Mappers::Discursive::toDomain(dto));
    }

    void setRecommendationTrajectoryDTO(const Application::DTO::RecommendationTrajectoryDTO& dto) {
        recommendationTrajectory_ = Application::Mappers::Recommendation::toDomain(dto);
    }

    void addRecommendationSnapshotDTO(const Application::DTO::RecommendationSnapshotDTO& dto) {
        if (recommendationTrajectory_.getId().empty()) {
            recommendationTrajectory_ = SisterSTRATA::Observational::Recommendation::RecommendationTrajectory(
                "REC-TRAJECTORY-1",
                {}
            );
        }
        recommendationTrajectory_.addSnapshot(Application::Mappers::Recommendation::toDomain(dto));
    }

    // --- CRUD Wrappers for Discursive System ---
    void removeDiscursiveSystemDTO(const std::string& id) {
        discursiveSystemRepository_->removeSystem(id);
    }

    void updateDiscursiveSystemDTO(const std::string& id, const Application::DTO::DiscursiveSystemDTO& dto) {
        discursiveSystemRepository_->updateSystem(id, Application::Mappers::Discursive::toDomain(dto));
    }

    // --- CRUD Wrappers for Narrative Observation ---
    void removeNarrativeStateDTO(const std::string& id) {
        narrativeSystem_->removeObservation(id);
    }

    void updateNarrativeStateDTO(const std::string& id, const Application::DTO::NarrativeStateDTO& dto) {
        narrativeSystem_->updateObservation(id, Application::Mappers::Narrative::toDomain(dto));
    }

    void loadDiscursiveSystemsFromFile(const std::string& path) {
        discursiveSystemRepository_->deserialize(path);
    }

    void saveDiscursiveSystemsToFile(const std::string& path) const {
        discursiveSystemRepository_->serialize(path);
    }

    void loadRecommendationTrajectoryFromFile(const std::string& path) {
        recommendationTrajectory_.deserialize(path);
    }

    void saveRecommendationTrajectoryToFile(const std::string& path) const {
        recommendationTrajectory_.serialize(path);
    }

    [[nodiscard]] size_t getDiscursiveSystemCount() const {
        return discursiveSystemRepository_->getSystems().size();
    }

    [[nodiscard]] size_t getRecommendationSnapshotCount() const {
        return recommendationTrajectory_.getSnapshots().size();
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
    std::unique_ptr<SisterSTRATA::Observational::Discursive::DiscursiveSystemRepository> discursiveSystemRepository_;
    SisterSTRATA::Observational::Recommendation::RecommendationTrajectory recommendationTrajectory_;
    Core::Domain::FourthDimension::Trajectory trajectory_;
    std::unique_ptr<Ports::ILLMService> llmService_;
};

} // namespace Application
