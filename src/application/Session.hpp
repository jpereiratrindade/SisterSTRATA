#pragma once

#include "core/domain/Workspace.hpp"
#include "core/domain/fourth_dimension/Trajectory.hpp"
#include "core/domain/identity/IdentityNode.hpp"
#include "core/domain/soil/SoilMonitorNode.hpp"
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
#include "core/domain/world/WorldState.hpp"
#include "application/ports/IWorldView.hpp"
#include "infrastructure/io/ObjLoader.hpp"
#include "infrastructure/io/CsvLoader.hpp"

// New extracted services
#include "application/services/IWIngestionService.hpp"
#include "application/services/ProjectPersistenceService.hpp"
#include "application/services/SimulationService.hpp"
#include "application/services/NarrativeContextAnalyzer.hpp"

#include <memory>
#include <vector>
#include <map>
#include <filesystem>
#include <iostream>
#include <nlohmann/json.hpp>

namespace Application {

enum class InfrastructureEcologicalScenario {
    Normal,
    SevereDrought
};

struct InfrastructureEvaluationConfig {
    int days{365};
    InfrastructureEcologicalScenario ecologicalScenario{InfrastructureEcologicalScenario::Normal};
    double identityEventsPerAnimalPerDay{20.0};
    strata::domain::identity::IdentityEnergyProfile identityProfile{
        .boot_wh_per_day = 0.5,
        .idle_wh_per_day = 4.5,
        .sensing_wh_per_event = 1.2,
        .processing_wh_per_event = 0.6,
        .communication_wh_per_event = 0.2
    };
    strata::domain::soil::SoilEnergyProfile soilProfile{
        .boot_wh_per_day = 0.5,
        .idle_wh_per_day = 7.5,
        .sensing_base_wh_per_day = 1.5,
        .communication_base_wh_per_day = 0.5,
        .dynamic_measurement_max_wh = 40.0
    };
    int ftNodeCount{1};
    double ftHardwareCostUsd{0.0};
    nlohmann::json ftComponentSelection{nlohmann::json::array()};
    std::string trigger{"analysis_workspace_manual_run"};
};

/**
 * @brief Application Session — thin facade delegating to dedicated services.
 *
 * After refactoring v1.9.8, Session retains:
 *   - Ownership of domain objects (workspace, repositories, trajectory)
 *   - CRUD wrappers the UI calls (register/update/remove DTOs)
 *   - World loading (3D data is loaded here, not in ingestion)
 *   - LLM / Cognitive wiring
 *   - Impact profile generation
 *
 * Delegated to:
 *   - IWIngestionService:         all IW ingestion logic
 *   - ProjectPersistenceService:  auto-save/load
 *   - SimulationService:          landscape simulation
 *   - NarrativeContextAnalyzer:   tokenization and context graph
 */
class Session {
public:
    // Persistence Config
    std::filesystem::path projectRoot_ = "assets/data/user_db";

    // Keep SimulationType alias for backward compatibility with UI code
    using SimulationType = Services::SimulationService::SimulationType;

    Session()
        : workspace_(std::make_unique<Core::Domain::Workspace>()),
          narrativeSystem_(std::make_unique<SisterSTRATA::Observational::Narrative::NarrativeObservationSystem>()),
          discursiveSystemRepository_(std::make_unique<SisterSTRATA::Observational::Discursive::DiscursiveSystemRepository>()),
          interpretationRepository_(std::make_unique<SisterSTRATA::Observational::Interpretation::InterpretationRepository>()),
          llmService_(nullptr),
          cognitiveService_(std::make_unique<Application::Services::Cognitive::CognitiveAssistanceService>(nullptr)),
          worldState_(std::make_unique<Core::Domain::WorldState>())
    {
        std::filesystem::create_directories(projectRoot_);
        initServices();
        persistenceService_->initializePersistence();
        ingestionService_->scanForIngestion();
    }

    void setProjectRoot(const std::string& path) {
        projectRoot_ = path;
        std::filesystem::create_directories(projectRoot_);
        newSession();
        initServices();
        persistenceService_->initializePersistence();
        ingestionService_->scanForIngestion();
    }

    [[nodiscard]] std::string getProjectRoot() const {
        return projectRoot_.string();
    }

    [[nodiscard]] Core::Domain::Workspace& getWorkspace() const {
        return *workspace_;
    }

    void newSession() {
        workspace_ = std::make_unique<Core::Domain::Workspace>();
        narrativeSystem_ = std::make_unique<SisterSTRATA::Observational::Narrative::NarrativeObservationSystem>();
        discursiveSystemRepository_ = std::make_unique<SisterSTRATA::Observational::Discursive::DiscursiveSystemRepository>();
        interpretationRepository_ = std::make_unique<SisterSTRATA::Observational::Interpretation::InterpretationRepository>();
        recommendationTrajectory_ = SisterSTRATA::Observational::Recommendation::RecommendationTrajectory();
        trajectory_.clear();
        initServices();
    }

    [[nodiscard]] Core::Domain::FourthDimension::Trajectory& getTrajectory() {
        return trajectory_;
    }

    [[nodiscard]] SisterSTRATA::Observational::Narrative::NarrativeObservationSystem& getNarrativeSystem() const {
        return *narrativeSystem_;
    }

    // ─────────────────────────────────────────────
    //  Narrative CRUD
    // ─────────────────────────────────────────────

    [[nodiscard]] std::vector<Application::DTO::NarrativeStateDTO> getNarrativeHistoryDTO() const {
        std::vector<Application::DTO::NarrativeStateDTO> dtos;
        for (const auto& state : narrativeSystem_->getHistory()) {
            dtos.push_back(Application::Mappers::Narrative::toDTO(state));
        }
        return dtos;
    }

    [[nodiscard]] nlohmann::json getNarrativeContextGraph() const {
        return Services::NarrativeContextAnalyzer::buildContextGraph(getNarrativeHistoryDTO());
    }

    void registerNarrativeStateDTO(const Application::DTO::NarrativeStateDTO& dto) {
        narrativeSystem_->registerObservation(Application::Mappers::Narrative::toDomain(dto));
        persistenceService_->autoSaveNarrative();
    }

    void loadNarrativeFromFile(const std::string& path) {
        narrativeSystem_->deserialize(path);
        persistenceService_->autoSaveNarrative();
    }

    void saveNarrativeToFile(const std::string& path) const {
        narrativeSystem_->serialize(path);
    }

    void removeNarrativeStateDTO(const std::string& id) {
        narrativeSystem_->removeObservation(id);
        persistenceService_->autoSaveNarrative();
    }

    void updateNarrativeStateDTO(const std::string& id, const Application::DTO::NarrativeStateDTO& dto) {
        narrativeSystem_->updateObservation(id, Application::Mappers::Narrative::toDomain(dto));
        persistenceService_->autoSaveNarrative();
    }

    // ─────────────────────────────────────────────
    //  Discursive CRUD
    // ─────────────────────────────────────────────

    [[nodiscard]] SisterSTRATA::Observational::Discursive::DiscursiveSystemRepository& getDiscursiveSystemRepository() const {
        return *discursiveSystemRepository_;
    }

    [[nodiscard]] std::vector<Application::DTO::DiscursiveSystemDTO> getDiscursiveSystemDTOs() const {
        std::vector<Application::DTO::DiscursiveSystemDTO> dtos;
        for (const auto& system : discursiveSystemRepository_->getSystems()) {
            dtos.push_back(Application::Mappers::Discursive::toDTO(system));
        }
        return dtos;
    }

    [[nodiscard]] size_t getDiscursiveSystemCount() const {
        return discursiveSystemRepository_->getSystems().size();
    }

    void registerDiscursiveSystemDTO(const Application::DTO::DiscursiveSystemDTO& dto) {
        discursiveSystemRepository_->registerSystem(Application::Mappers::Discursive::toDomain(dto));
        persistenceService_->autoSaveDiscursive();
    }

    void removeDiscursiveSystemDTO(const std::string& id) {
        discursiveSystemRepository_->removeSystem(id);
        persistenceService_->autoSaveDiscursive();
    }

    void updateDiscursiveSystemDTO(const std::string& id, const Application::DTO::DiscursiveSystemDTO& dto) {
        discursiveSystemRepository_->updateSystem(id, Application::Mappers::Discursive::toDomain(dto));
        persistenceService_->autoSaveDiscursive();
    }

    void loadDiscursiveSystemsFromFile(const std::string& path) {
        discursiveSystemRepository_->deserialize(path);
        persistenceService_->autoSaveDiscursive();
    }

    void saveDiscursiveSystemsToFile(const std::string& path) const {
        discursiveSystemRepository_->serialize(path);
    }

    // ─────────────────────────────────────────────
    //  Recommendation CRUD
    // ─────────────────────────────────────────────

    [[nodiscard]] SisterSTRATA::Observational::Recommendation::RecommendationTrajectory& getRecommendationTrajectory() {
        return recommendationTrajectory_;
    }

    [[nodiscard]] Application::DTO::RecommendationTrajectoryDTO getRecommendationTrajectoryDTO() const {
        return Application::Mappers::Recommendation::toDTO(recommendationTrajectory_);
    }

    [[nodiscard]] size_t getRecommendationSnapshotCount() const {
        return recommendationTrajectory_.getSnapshots().size();
    }

    void setRecommendationTrajectoryDTO(const Application::DTO::RecommendationTrajectoryDTO& dto) {
        recommendationTrajectory_ = Application::Mappers::Recommendation::toDomain(dto);
    }

    void addRecommendationSnapshotDTO(const Application::DTO::RecommendationSnapshotDTO& dto) {
        if (recommendationTrajectory_.getId().empty()) {
            recommendationTrajectory_ = SisterSTRATA::Observational::Recommendation::RecommendationTrajectory(
                "REC-TRAJECTORY-1", {}
            );
        }
        recommendationTrajectory_.addSnapshot(Application::Mappers::Recommendation::toDomain(dto));
        persistenceService_->autoSaveRecommendation();
    }

    void removeRecommendationSnapshotDTO(const std::string& id) {
        recommendationTrajectory_.removeSnapshot(id);
        persistenceService_->autoSaveRecommendation();
    }

    void updateRecommendationSnapshotDTO(const std::string& id, const Application::DTO::RecommendationSnapshotDTO& dto) {
        recommendationTrajectory_.updateSnapshot(id, Application::Mappers::Recommendation::toDomain(dto));
        persistenceService_->autoSaveRecommendation();
    }

    void loadRecommendationTrajectoryFromFile(const std::string& path) {
        recommendationTrajectory_.deserialize(path);
        persistenceService_->autoSaveRecommendation();
    }

    void saveRecommendationTrajectoryToFile(const std::string& path) const {
        recommendationTrajectory_.serialize(path);
    }

    // ─────────────────────────────────────────────
    //  IW Ingestion (delegated)
    // ─────────────────────────────────────────────

    void ingestFromIW(const std::string& filepath) {
        ingestionService_->ingestFromIW(filepath);
    }

    void ingestFromIWDirectory(const std::string& dirPath) {
        ingestionService_->ingestFromIWDirectory(dirPath);
    }

    void scanForIngestion() {
        ingestionService_->scanForIngestion();
    }

    // ─────────────────────────────────────────────
    //  LLM / Cognitive
    // ─────────────────────────────────────────────

    void setLLMService(std::unique_ptr<Ports::ILLMService> llmService) {
        llmService_ = std::move(llmService);
        if (cognitiveService_) {
            cognitiveService_->setLLMService(llmService_.get());
        }
    }

    [[nodiscard]] Ports::ILLMService* getLLMService() const {
        return llmService_.get();
    }

    void saveInterpretationSnapshotDTO(const Application::DTO::Cognitive::InterpretationSnapshotDTO& dto) {
        interpretationRepository_->addSnapshot(Application::Mappers::Interpretation::toDomain(dto));
        persistenceService_->autoSaveInterpretation();
    }

    [[nodiscard]] std::vector<Application::DTO::Cognitive::InterpretationSnapshotDTO> getInterpretationSnapshots() const {
        std::vector<Application::DTO::Cognitive::InterpretationSnapshotDTO> dtos;
        for (const auto& s : interpretationRepository_->getSnapshots()) {
            dtos.push_back(Application::Mappers::Interpretation::toDTO(s));
        }
        return dtos;
    }

    void requestAIInterpretation(const Application::DTO::Cognitive::ContextBundleDTO& bundle,
                                Application::Services::Cognitive::InterpretationMode mode,
                                Application::Services::Cognitive::CognitiveAssistanceService::SnapshotCallback callback) {
        cognitiveService_->interpret(bundle, mode, callback);
    }

    // ─────────────────────────────────────────────
    //  Impact Profile
    // ─────────────────────────────────────────────

    std::string generateImpactProfileText() const {
        const auto& worldPtr = workspace_->getWorld();
        if (!worldPtr) return "";
        auto* world = worldPtr.get();

        namespace IP = SisterSTRATA::Observational::ImpactProfile;
        IP::Infrastructure::TrajectoryImpactAnalyzerImpl analyzer(
            world->getResolution().width, world->getResolution().height);

        Core::Domain::FourthDimension::Trajectory referenceTrajectory;
        if (!trajectory_.getTimeSlices().empty()) {
            referenceTrajectory.addTimeSlice(trajectory_.getTimeSlices().front());
        }

        IP::Domain::ReferenceFrame context(
            IP::Domain::ReferenceType::Historical,
            "Baseline (Estado Inicial)",
            "REF-START"
        );

        auto profile = analyzer.analyze(trajectory_, referenceTrajectory, context);
        return Application::Mappers::ImpactProfileMapper::toNaturalLanguage(profile);
    }

    // ─────────────────────────────────────────────
    //  World Management
    // ─────────────────────────────────────────────

    void setWorldView(Ports::IWorldView* view) {
        worldView_ = view;
    }

    void loadWorld(const std::string& path) {
        std::cout << "[Session] Loading world from: " << path << std::endl;

        worldState_->clear();
        if (worldView_) worldView_->clear();

        std::string ext = std::filesystem::path(path).extension().string();

        if (ext == ".obj") {
            bool isPointCloud = false;
            auto data = Infrastructure::IO::ObjLoader::load(path, &isPointCloud);

            Core::Domain::WorldEntity entity;
            entity.type = isPointCloud ? "point_cloud" : "mesh";
            entity.id = path;

            entity.points.reserve(data.positions.size());
            entity.colors.reserve(data.colors.size());

            for(size_t i=0; i<data.positions.size(); ++i) {
                entity.points.push_back({
                    (double)data.positions[i].x,
                    (double)data.positions[i].y,
                    (double)data.positions[i].z
                });

                if (i < data.colors.size()) entity.colors.push_back(data.colors[i]);
                else entity.colors.push_back({0.8, 0.8, 0.8});
            }

            worldState_->addEntity(entity);

        } else if (ext == ".csv" || ext == ".xyz" || ext == ".txt") {
             auto data = Infrastructure::IO::CsvLoader::load(path);

             Core::Domain::WorldEntity entity;
             entity.type = "point_cloud";
             entity.id = path;
             entity.points = data.points;
             entity.colors = data.colors;

             worldState_->addEntity(entity);
        }

        if (worldView_) {
            worldView_->onWorldLoaded(*worldState_);
        }

        loadSidecarData(path);
    }

    void loadSidecarData(const std::string& path) {
         try {
            getNarrativeSystem().deserialize(path + ".json");
            std::cout << "[Session] Sidecar: Narrative loaded." << std::endl;
        } catch (...) {}
        try {
            getDiscursiveSystemRepository().deserialize(path + ".discursive.json");
            std::cout << "[Session] Sidecar: Discursive loaded." << std::endl;
        } catch (...) {}
        try {
            getRecommendationTrajectory().deserialize(path + ".recommendation.json");
            std::cout << "[Session] Sidecar: Recommendation loaded." << std::endl;
        } catch (...) {}
    }

    // ─────────────────────────────────────────────
    //  Simulation (delegated)
    // ─────────────────────────────────────────────

    void simulateCondition(SimulationType type) {
        Services::SimulationService::simulateCondition(type, *workspace_, trajectory_);
    }

    /**
     * @brief Runs InfrastructureLayer v0.1 resilience simulation and writes report artifacts.
     * @param days Number of discrete daily steps.
     * @return Path to latest JSON report file, or empty on failure.
     */
    std::string runInfrastructureResilienceSimulation(int days = 365);
    std::string runInfrastructureResilienceSimulation(const InfrastructureEvaluationConfig& config);

private:
    void initServices() {
        persistenceService_ = std::make_unique<Services::ProjectPersistenceService>(
            projectRoot_,
            *narrativeSystem_,
            *discursiveSystemRepository_,
            recommendationTrajectory_,
            *interpretationRepository_
        );
        ingestionService_ = std::make_unique<Services::IWIngestionService>(
            *narrativeSystem_,
            *discursiveSystemRepository_,
            recommendationTrajectory_,
            *persistenceService_,
            projectRoot_
        );
    }

    // Domain objects
    std::unique_ptr<Core::Domain::Workspace> workspace_;
    std::unique_ptr<SisterSTRATA::Observational::Narrative::NarrativeObservationSystem> narrativeSystem_;
    std::unique_ptr<SisterSTRATA::Observational::Discursive::DiscursiveSystemRepository> discursiveSystemRepository_;
    std::unique_ptr<SisterSTRATA::Observational::Interpretation::InterpretationRepository> interpretationRepository_;
    std::unique_ptr<Ports::ILLMService> llmService_;
    std::unique_ptr<Application::Services::Cognitive::CognitiveAssistanceService> cognitiveService_;
    SisterSTRATA::Observational::Recommendation::RecommendationTrajectory recommendationTrajectory_;
    Core::Domain::FourthDimension::Trajectory trajectory_;

    // World
    std::unique_ptr<Core::Domain::WorldState> worldState_;
    Ports::IWorldView* worldView_ = nullptr;

    // Extracted services
    std::unique_ptr<Services::ProjectPersistenceService> persistenceService_;
    std::unique_ptr<Services::IWIngestionService> ingestionService_;
};

} // namespace Application
