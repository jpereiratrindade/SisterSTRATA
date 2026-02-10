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
#include "src/application/mappers/ImpactProfileMapper.hpp"
// #include "world3d/World3D.hpp" // Removed for architectural purity (Headless)
#include "core/domain/world/WorldState.hpp"
#include "application/ports/IWorldView.hpp"
#include "infrastructure/io/ObjLoader.hpp"
#include "infrastructure/io/CsvLoader.hpp"
#include "infrastructure/io/CsvLoader.hpp"
#include "src/application/mappers/IWMapper.hpp"
#include <memory>
#include <vector>
#include <filesystem>
#include <iostream>
#include <fstream>

namespace Application {

class Session {
public:
    // Persistence Config
    std::filesystem::path projectRoot_ = "assets/data/user_db";

    Session() 
        : workspace_(std::make_unique<Core::Domain::Workspace>()),
          narrativeSystem_(std::make_unique<SisterSTRATA::Observational::Narrative::NarrativeObservationSystem>()),
          discursiveSystemRepository_(std::make_unique<SisterSTRATA::Observational::Discursive::DiscursiveSystemRepository>()),
          interpretationRepository_(std::make_unique<SisterSTRATA::Observational::Interpretation::InterpretationRepository>()),
          llmService_(nullptr),
          cognitiveService_(std::make_unique<Application::Services::Cognitive::CognitiveAssistanceService>(nullptr)),
          worldState_(std::make_unique<Core::Domain::WorldState>()) // Init WorldState
    {
        // Ensure standard persistence directories exist
        std::filesystem::create_directories(projectRoot_);
        initializePersistence();
    }

    void setProjectRoot(const std::string& path) {
        projectRoot_ = path;
        std::filesystem::create_directories(projectRoot_);
        
        // Reload data for the new project
        // 1. Clear current state
        newSession();
        
        // 2. Load from new path
        initializePersistence();
    }

    [[nodiscard]] std::string getProjectRoot() const {
        return projectRoot_.string();
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

    void ingestFromIW(const std::string& filepath) {
        std::cout << "[Session] Ingesting from IW: " << filepath << std::endl;
        try {
            std::ifstream f(filepath);
            if (!f.is_open()) {
                std::cerr << "Failed to open IW file: " << filepath << std::endl;
                return;
            }
            nlohmann::json j;
            f >> j;

            // 1. Discursive System
            auto discDTO = Application::Mappers::IW::IWMapper::toDiscursiveSystemDTO(j);
            // Generate a unique ID if empty
            if (discDTO.id.empty()) {
                discDTO.id = "DS-IW-" + std::to_string(getDiscursiveSystemCount() + 1);
            }
            // Only register if it has content
            if (!discDTO.declaredProblems.empty() || !discDTO.declaredActions.empty()) {
                 registerDiscursiveSystemDTO(discDTO);
                 std::cout << " -> Ingested Discursive System: " << discDTO.id << std::endl;
            }

            // 2. Narrative Observations
            auto narrDTOs = Application::Mappers::IW::IWMapper::toNarrativeStateDTOs(j);
            for (auto& narrDTO : narrDTOs) {
                if (narrDTO.id.empty()) {
                    narrDTO.id = "OBS-IW-" + std::to_string(narrativeSystem_->getHistory().size() + 1);
                }
                registerNarrativeStateDTO(narrDTO);
            }
            if (!narrDTOs.empty()) {
                std::cout << " -> Ingested " << narrDTOs.size() << " Narrative Observations." << std::endl;
            }

            // 3. Recommendation Snapshot (Trajectory Analogy)
            auto recOpt = Application::Mappers::IW::IWMapper::toRecommendationSnapshotDTO(j);
            if (recOpt.has_value()) {
                auto recDTO = recOpt.value();
                recDTO.id = "REC-IW-" + std::to_string(getRecommendationSnapshotCount() + 1);
                addRecommendationSnapshotDTO(recDTO);
                std::cout << " -> Ingested Recommendation Snapshot: " << recDTO.id << std::endl;
            }

        } catch (const std::exception& e) {
            std::cerr << "Error ingesting IW file: " << e.what() << std::endl;
        }
    }

    void ingestFromIWDirectory(const std::string& dirPath) {
        std::cout << "[Session] Batch Ingestion from Directory: " << dirPath << std::endl;
        if (!std::filesystem::exists(dirPath) || !std::filesystem::is_directory(dirPath)) {
            std::cerr << "Invalid directory for ingestion: " << dirPath << std::endl;
            return;
        }

        for (const auto& entry : std::filesystem::recursive_directory_iterator(dirPath)) {
            if (entry.is_regular_file() && entry.path().extension() == ".json") {
                ingestFromIW(entry.path().string());
            }
        }
        std::cout << "[Session] Batch Ingestion Complete." << std::endl;
    }

    void scanForIngestion() {
        std::cout << "[Session] Scanning project for inputs..." << std::endl;
        
        std::filesystem::path inputsDir = projectRoot_ / "inputs";
        if (!std::filesystem::exists(inputsDir)) {
            // Create structure if it doesn't exist to guide user
            try {
                std::filesystem::create_directories(inputsDir / "narratives");
                std::filesystem::create_directories(inputsDir / "discursive");
                std::cout << "[Session] Created input directories at " << inputsDir << std::endl;
            } catch (...) {}
            return;
        }

        auto scanDir = [&](const std::filesystem::path& dir) {
             if (!std::filesystem::exists(dir)) return;
             for (const auto& entry : std::filesystem::directory_iterator(dir)) {
                 if (entry.is_regular_file() && entry.path().extension() == ".json") {
                     ingestFromIW(entry.path().string());
                 }
             }
        };

        scanDir(inputsDir / "narratives");
        scanDir(inputsDir / "discursive");
        // We could scan recursively or other folders, but sticking to specific input folders is cleaner.
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

    // --- World Management ---
    
    void setWorldView(Ports::IWorldView* view) {
        worldView_ = view;
    }

    void loadWorld(const std::string& path) {
        std::cout << "[Session] Loading world from: " << path << std::endl;
        
        // 1. Clear previous
        worldState_->clear();
        if (worldView_) worldView_->clear();

        // 2. Determine Loader
        std::string ext = std::filesystem::path(path).extension().string();
        
        if (ext == ".obj") {
            bool isPointCloud = false;
            auto data = Infrastructure::IO::ObjLoader::load(path, &isPointCloud);
            
            Core::Domain::WorldEntity entity;
            entity.type = isPointCloud ? "point_cloud" : "mesh";
            entity.id = path;
            
            // Map Infrastructure Data -> Domain IO (or just copy)
            // WorldState uses Core::ValueObjects::Vector3 (double)
            // ObjLoader uses glm::vec3 (float)
            // Need conversion.
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
             // Basic Check if polyline or pointcloud
             // For now assume pointcloud unless specialized loader called
             // CsvLoader handles this internally via different methods?
             // Let's use the generic load() for points.
             auto data = Infrastructure::IO::CsvLoader::load(path);
             
             Core::Domain::WorldEntity entity;
             entity.type = "point_cloud";
             entity.id = path;
             entity.points = data.points; // Direct copy (Vector3)
             entity.colors = data.colors;
             
             worldState_->addEntity(entity);
        }
        
        // 3. Notify View
        if (worldView_) {
            worldView_->onWorldLoaded(*worldState_);
        }
        
        // 4. Load Sidecars
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
            
            // Fix: Create physical mesh for visualization (Ghost Mode requires vertices)
            // Use Type::Showcase (4) to ensure terrain has slopes/drainage context, avoiding "All Green" flat fallback.
            // TODO: Move Terrain Generation to Core/Infrastructure
            // World3D::generateTerrain("simulation_mesh.obj", 100, 100, 2.0f, 4, true);
            std::cerr << "[Session] Warning: Terrain Generation currently unavailable in Headless/Session context." << std::endl;
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
                 ordinal, ordinal, cover, water, meta,
                 Core::Domain::FourthDimension::ClassificationType::SemanticCode
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
                    // Use -1 (Soil) for non-forest to make it visually distinct (Brown cs Dark Green)
                    // 0 is Campestre (Light Green), which looks too similar to Forest (Dark Green)
                    fragmented[y*w + x] = ((x/10 + y/10) % 2 == 0) ? 1 : -1;
                }
            }
            addSlice(2, fragmented, "Simulated Fragmentation");
        }
        else if (type == SimulationType::Deforestation) {
            addSlice(1, baseline, "Baseline (Intact)");
            
            // Massive loss (80% gone)
            // Use -1 (Soil) instead of 0 (Campestre) for high contrast visualization
            std::vector<int> deforested(size, -1); 
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
    std::unique_ptr<Ports::ILLMService> llmService_;
    std::unique_ptr<Application::Services::Cognitive::CognitiveAssistanceService> cognitiveService_;
    SisterSTRATA::Observational::Recommendation::RecommendationTrajectory recommendationTrajectory_;
    Core::Domain::FourthDimension::Trajectory trajectory_;

    // Persistence Helpers
    // Persistence Helpers
    void initializePersistence() {
        std::filesystem::path discursivePath = projectRoot_ / "discursive_systems.json";
        if (std::filesystem::exists(discursivePath)) {
            try {
                discursiveSystemRepository_->deserialize(discursivePath.string());
            } catch (...) {
                // Ignore load errors on init, start fresh or log
            }
        }

        std::filesystem::path narrativePath = projectRoot_ / "narrative_history.json";
        if (std::filesystem::exists(narrativePath)) {
            try {
                narrativeSystem_->deserialize(narrativePath.string());
            } catch (...) {
                // Ignore load errors on init
            }
        }

        std::filesystem::path recPath = projectRoot_ / "recommendation_trajectory.json";
        if (std::filesystem::exists(recPath)) {
            try {
                recommendationTrajectory_.deserialize(recPath.string());
            } catch (...) {}
        }
        
        std::filesystem::path interpPath = projectRoot_ / "interpretation_memory.json";
        if (std::filesystem::exists(interpPath)) {
            try {
                interpretationRepository_->deserialize(interpPath.string());
            } catch (...) {}
        }
        
        // Auto-ingest inputs on load
        scanForIngestion();
    }

    void autoSaveDiscursive() {
        try {
            discursiveSystemRepository_->serialize((projectRoot_ / "discursive_systems.json").string());
        } catch (...) {}
    }

    void autoSaveNarrative() {
         try {
            narrativeSystem_->serialize((projectRoot_ / "narrative_history.json").string());
        } catch (...) {}
    }

    void autoSaveRecommendation() {
        try {
            recommendationTrajectory_.serialize((projectRoot_ / "recommendation_trajectory.json").string());
        } catch (...) {}
    }

    void autoSaveInterpretation() {
        try {
            interpretationRepository_->serialize((projectRoot_ / "interpretation_memory.json").string());
        } catch (...) {}
    }
    
    // World State Owner
    std::unique_ptr<Core::Domain::WorldState> worldState_;
    Ports::IWorldView* worldView_ = nullptr;
};

} // namespace Application
