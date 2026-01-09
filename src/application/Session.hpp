#pragma once

#include "core/domain/Workspace.hpp"
#include "core/domain/fourth_dimension/Trajectory.hpp"
#include "application/ports/ILLMService.hpp"
#include "application/dtos/DiscursiveSystemDTO.hpp"
#include "application/dtos/RecommendationSnapshotDTO.hpp"
#include "application/dtos/RecommendationTrajectoryDTO.hpp"
#include "application/dtos/NarrativeDTOs.hpp"
#include "src/application/mappers/InterpretationMapper.hpp"
#include "src/application/dtos/cognitive/ContextBundleDTO.hpp"
#include "src/application/services/cognitive/CognitiveAssistanceService.hpp"
#include "application/mappers/ObservationalMappers.hpp"
#include "src/observational/narrative/aggregates/NarrativeObservationSystem.hpp"
#include "src/observational/discursive/aggregates/DiscursiveSystemRepository.hpp"
#include "src/observational/recommendation/aggregates/RecommendationTrajectory.hpp"
#include "src/observational/interpretation/aggregates/InterpretationRepository.hpp"
#include "src/observational/impact_profile/infrastructure/TrajectoryImpactAnalyzerImpl.hpp"
#include "src/application/mappers/ImpactProfileMapper.hpp"
#include <memory>
#include <vector>

namespace Application {

class Session {
public:
    // Persistence Config
    inline static const std::string DISCURSIVE_DB_PATH = "assets/data/user_db/discursive_systems.json";
    inline static const std::string NARRATIVE_DB_PATH = "assets/data/user_db/narrative_history.json";
    inline static const std::string RECOMMENDATION_DB_PATH = "assets/data/user_db/recommendation_trajectory.json";
    inline static const std::string INTERPRETATION_DB_PATH = "assets/data/user_db/interpretation_memory.json";

    Session() 
        : workspace_(std::make_unique<Core::Domain::Workspace>()),
          narrativeSystem_(std::make_unique<SisterSTRATA::Observational::Narrative::NarrativeObservationSystem>()),
          discursiveSystemRepository_(std::make_unique<SisterSTRATA::Observational::Discursive::DiscursiveSystemRepository>()),
          interpretationRepository_(std::make_unique<SisterSTRATA::Observational::Interpretation::InterpretationRepository>()),
          cognitiveService_(std::make_unique<Application::Services::Cognitive::CognitiveAssistanceService>(getLLMService()))
    {
        initializePersistence();
    }

    [[nodiscard]] Core::Domain::Workspace& getWorkspace() const {
        return *workspace_;
    }

    // Ability to reset session if needed
    void newSession() {
        workspace_ = std::make_unique<Core::Domain::Workspace>();
        narrativeSystem_ = std::make_unique<SisterSTRATA::Observational::Narrative::NarrativeObservationSystem>();
        discursiveSystemRepository_ = std::make_unique<SisterSTRATA::Observational::Discursive::DiscursiveSystemRepository>();
        interpretationRepository_ = std::make_unique<SisterSTRATA::Observational::Interpretation::InterpretationRepository>();
        recommendationTrajectory_ = SisterSTRATA::Observational::Recommendation::RecommendationTrajectory();
        trajectory_.clear();
    }

    [[nodiscard]] Core::Domain::FourthDimension::Trajectory& getTrajectory() {
        return trajectory_;
    }

    [[nodiscard]] SisterSTRATA::Observational::Narrative::NarrativeObservationSystem& getNarrativeSystem() const {
        return *narrativeSystem_;
    }

    [[nodiscard]] std::vector<Application::DTO::NarrativeStateDTO> getNarrativeHistoryDTO() const {
        std::vector<Application::DTO::NarrativeStateDTO> dtos;
        for (const auto& state : narrativeSystem_->getHistory()) {
            dtos.push_back(Application::Mappers::Narrative::toDTO(state));
        }
        return dtos;
    }

    void registerNarrativeStateDTO(const Application::DTO::NarrativeStateDTO& dto) {
        narrativeSystem_->registerObservation(Application::Mappers::Narrative::toDomain(dto));
        autoSaveNarrative();
    }

    void loadNarrativeFromFile(const std::string& path) {
        narrativeSystem_->deserialize(path);
        autoSaveNarrative();
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
        autoSaveDiscursive();
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
        autoSaveRecommendation();
    }

    // --- CRUD Wrappers for Recommendation Snapshot ---
    void removeRecommendationSnapshotDTO(const std::string& id) {
        recommendationTrajectory_.removeSnapshot(id);
        autoSaveRecommendation();
    }

    void updateRecommendationSnapshotDTO(const std::string& id, const Application::DTO::RecommendationSnapshotDTO& dto) {
        recommendationTrajectory_.updateSnapshot(id, Application::Mappers::Recommendation::toDomain(dto));
        autoSaveRecommendation();
    }

    // --- CRUD Wrappers for Discursive System ---
    void removeDiscursiveSystemDTO(const std::string& id) {
        discursiveSystemRepository_->removeSystem(id);
        autoSaveDiscursive();
    }

    void updateDiscursiveSystemDTO(const std::string& id, const Application::DTO::DiscursiveSystemDTO& dto) {
        discursiveSystemRepository_->updateSystem(id, Application::Mappers::Discursive::toDomain(dto));
        autoSaveDiscursive();
    }

    // --- CRUD Wrappers for Narrative Observation ---
    void removeNarrativeStateDTO(const std::string& id) {
        narrativeSystem_->removeObservation(id);
        autoSaveNarrative();
    }

    void updateNarrativeStateDTO(const std::string& id, const Application::DTO::NarrativeStateDTO& dto) {
        narrativeSystem_->updateObservation(id, Application::Mappers::Narrative::toDomain(dto));
        autoSaveNarrative();
    }

    void loadDiscursiveSystemsFromFile(const std::string& path) {
        discursiveSystemRepository_->deserialize(path);
        autoSaveDiscursive();
    }

    void saveDiscursiveSystemsToFile(const std::string& path) const {
        discursiveSystemRepository_->serialize(path);
    }

    void loadRecommendationTrajectoryFromFile(const std::string& path) {
        recommendationTrajectory_.deserialize(path);
        autoSaveRecommendation();
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
        if (cognitiveService_) {
            cognitiveService_->setLLMService(llmService_.get());
        }
    }

    [[nodiscard]] Ports::ILLMService* getLLMService() const {
        return llmService_.get();
    }

    // --- Cognitive Interpretation Memory ---
    void saveInterpretationSnapshotDTO(const Application::DTO::Cognitive::InterpretationSnapshotDTO& dto) {
        interpretationRepository_->addSnapshot(Application::Mappers::Interpretation::toDomain(dto));
        autoSaveInterpretation();
    }

    [[nodiscard]] std::vector<Application::DTO::Cognitive::InterpretationSnapshotDTO> getInterpretationSnapshots() const {
        std::vector<Application::DTO::Cognitive::InterpretationSnapshotDTO> dtos;
        for (const auto& s : interpretationRepository_->getSnapshots()) {
            dtos.push_back(Application::Mappers::Interpretation::toDTO(s));
        }
        return dtos;
    }

    // --- Cognitive Assistance ---
    void requestAIInterpretation(const Application::DTO::Cognitive::ContextBundleDTO& bundle,
                                Application::Services::Cognitive::InterpretationMode mode,
                                Application::Services::Cognitive::CognitiveAssistanceService::SnapshotCallback callback) {
        cognitiveService_->interpret(bundle, mode, callback);
    }

    // --- Trajectory Impact Analysis ---
    std::string generateImpactProfileText() const {
        // 1. Check Prerequisites
        const auto& worldPtr = workspace_->getWorld();
        if (!worldPtr) return "";
        auto* world = worldPtr.get();

        // 2. Instantiate Analyzer (Infrastructure)
        using namespace SisterSTRATA::Observational::ImpactProfile;
        SisterSTRATA::Observational::ImpactProfile::Infrastructure::TrajectoryImpactAnalyzerImpl analyzer(world->getResolution().width, world->getResolution().height);

        // 3. Define Reference (Self-Reference: First Frame vs Last Frame)
        // If trajectory has < 2 slices, we can't really do a trend, but can do a structure check if referencing a theoretical baseline.
        // For now, we try to grab the FIRST slice as the "Historical Baseline".
        Core::Domain::FourthDimension::Trajectory referenceTrajectory;
        if (!trajectory_.getTimeSlices().empty()) {
            referenceTrajectory.addTimeSlice(trajectory_.getTimeSlices().front());
        }

        Domain::ReferenceFrame context(
            Domain::ReferenceType::Historical,
            "Baseline (Estado Inicial)",
            "REF-START"
        );

        // 4. Analyze
        auto profile = analyzer.analyze(trajectory_, referenceTrajectory, context);

        // 5. Map to Natural Language
        return Application::Mappers::ImpactProfileMapper::toNaturalLanguage(profile);
    }

    // --- Simulation Tools ---
    enum class SimulationType {
        Stability,
        Fragmentation,
        Deforestation
    };

    void simulateCondition(SimulationType type) {
        auto* world = workspace_->getWorld().get();
        
        // Auto-create world if missing (Mock for Simulation)
        if (!world) {
            workspace_->createWorld("Simulation Environment", 100, 100);
            world = workspace_->getWorld().get();
        }

        if (!world) return; // Should not happen

        trajectory_.clear(); 

        int w = world->getResolution().width;
        int h = world->getResolution().height;
        size_t size = w * h;

        // Helper to generate a slice
        auto addSlice = [&](int ordinal, const std::vector<int>& cover, const std::string& meta) {
             std::vector<bool> water(size, false);
             trajectory_.addTimeSlice(Core::Domain::FourthDimension::TimeSlice(
                 ordinal, ordinal, cover, water, meta
             ));
        };

        std::vector<int> baseline(size, 1); // Full Forest

        if (type == SimulationType::Stability) {
            addSlice(1, baseline, "Baseline Year 1");
            addSlice(2, baseline, "Baseline Year 5"); // Identical
        }
        else if (type == SimulationType::Fragmentation) {
            addSlice(1, baseline, "Baseline (Intact)");
            
            // Checkerboard pattern (High Fragmentation)
            std::vector<int> fragmented(size);
            for(int y=0; y<h; ++y) {
                for(int x=0; x<w; ++x) {
                    fragmented[y*w + x] = ((x/10 + y/10) % 2 == 0) ? 1 : 0;
                }
            }
            addSlice(2, fragmented, "Simulated Fragmentation");
        }
        else if (type == SimulationType::Deforestation) {
            addSlice(1, baseline, "Baseline (Intact)");
            
            // Massive loss (80% gone)
            std::vector<int> deforested(size, 0); 
            // Keep small patch in corner
            for(int i=0; i<size/10; ++i) deforested[i] = 1;
            
            addSlice(2, deforested, "Simulated Deforestation");
        }
    }

private:
    std::unique_ptr<Core::Domain::Workspace> workspace_;
    std::unique_ptr<SisterSTRATA::Observational::Narrative::NarrativeObservationSystem> narrativeSystem_;
    std::unique_ptr<SisterSTRATA::Observational::Discursive::DiscursiveSystemRepository> discursiveSystemRepository_;
    std::unique_ptr<SisterSTRATA::Observational::Interpretation::InterpretationRepository> interpretationRepository_;
    std::unique_ptr<Application::Services::Cognitive::CognitiveAssistanceService> cognitiveService_;
    SisterSTRATA::Observational::Recommendation::RecommendationTrajectory recommendationTrajectory_;
    Core::Domain::FourthDimension::Trajectory trajectory_;
    std::unique_ptr<Ports::ILLMService> llmService_;

    // Persistence Helpers
    void initializePersistence() {
        if (std::filesystem::exists(DISCURSIVE_DB_PATH)) {
            try {
                discursiveSystemRepository_->deserialize(DISCURSIVE_DB_PATH);
            } catch (...) {
                // Ignore load errors on init, start fresh or log
            }
        }
        if (std::filesystem::exists(NARRATIVE_DB_PATH)) {
            try {
                narrativeSystem_->deserialize(NARRATIVE_DB_PATH);
            } catch (...) {
                // Ignore load errors on init
            }
        }
        if (std::filesystem::exists(RECOMMENDATION_DB_PATH)) {
            try {
                recommendationTrajectory_.deserialize(RECOMMENDATION_DB_PATH);
            } catch (...) {}
        }
        if (std::filesystem::exists(INTERPRETATION_DB_PATH)) {
            try {
                interpretationRepository_->deserialize(INTERPRETATION_DB_PATH);
            } catch (...) {}
        }
    }

    void autoSaveDiscursive() {
        try {
            discursiveSystemRepository_->serialize(DISCURSIVE_DB_PATH);
        } catch (...) {}
    }

    void autoSaveNarrative() {
         try {
            narrativeSystem_->serialize(NARRATIVE_DB_PATH);
        } catch (...) {}
    }

    void autoSaveRecommendation() {
        try {
            recommendationTrajectory_.serialize(RECOMMENDATION_DB_PATH);
        } catch (...) {}
    }

    void autoSaveInterpretation() {
        try {
            interpretationRepository_->serialize(INTERPRETATION_DB_PATH);
        } catch (...) {}
    }
};

} // namespace Application
