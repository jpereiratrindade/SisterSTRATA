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
#include <map>
#include <set>
#include <array>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cctype>
#include <optional>
#include <nlohmann/json.hpp>

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
        std::filesystem::path filePath(filepath);
        if (!std::filesystem::exists(filePath) || !std::filesystem::is_regular_file(filePath)) {
            std::cerr << "Invalid IW file: " << filepath << std::endl;
            return;
        }

        IWIngestSummary summary;
        if (isIWBundleDirectory(filePath.parent_path())) {
            summary.bundlesDetected = 1;
            ingestIWBundleDirectory(filePath.parent_path(), summary);
        } else {
            ingestIWStandaloneFile(filePath, summary);
        }
        logIWIngestSummary(summary);
    }

    void ingestFromIWDirectory(const std::string& dirPath) {
        std::cout << "[Session] Batch Ingestion from Directory: " << dirPath << std::endl;
        std::filesystem::path root(dirPath);
        if (!std::filesystem::exists(root) || !std::filesystem::is_directory(root)) {
            std::cerr << "Invalid directory for ingestion: " << dirPath << std::endl;
            return;
        }

        std::set<std::filesystem::path> bundleDirs;
        std::vector<std::filesystem::path> standaloneFiles;

        for (const auto& entry : std::filesystem::recursive_directory_iterator(root)) {
            if (!entry.is_regular_file() || entry.path().extension() != ".json") {
                continue;
            }

            const auto parent = entry.path().parent_path();
            if (isIWBundleDirectory(parent)) {
                bundleDirs.insert(parent);
            } else {
                standaloneFiles.push_back(entry.path());
            }
        }

        IWIngestSummary summary;
        summary.bundlesDetected = bundleDirs.size();

        for (const auto& bundleDir : bundleDirs) {
            ingestIWBundleDirectory(bundleDir, summary);
        }

        // If canonical IW bundles exist, they are the preferred source and we skip
        // standalone JSON files to avoid duplicate ingestion noise.
        if (bundleDirs.empty()) {
            std::sort(standaloneFiles.begin(), standaloneFiles.end());
            for (const auto& file : standaloneFiles) {
                ingestIWStandaloneFile(file, summary);
            }
        }

        logIWIngestSummary(summary);
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
    using json = nlohmann::json;

    struct IWIngestSummary {
        size_t bundlesDetected = 0;
        size_t bundlesIngested = 0;
        size_t standaloneFiles = 0;
        size_t discursiveMapped = 0;
        size_t discursiveSkipped = 0;
        size_t narrativeMapped = 0;
        size_t narrativeSkipped = 0;
        size_t recommendationMapped = 0;
        size_t recommendationSkipped = 0;
    };

    static std::string toMetadataValue(const json& value) {
        if (value.is_string()) return value.get<std::string>();
        if (value.is_number_integer()) return std::to_string(value.get<long long>());
        if (value.is_number_unsigned()) return std::to_string(value.get<unsigned long long>());
        if (value.is_number_float()) return std::to_string(value.get<double>());
        if (value.is_boolean()) return value.get<bool>() ? "true" : "false";
        if (value.is_null()) return "";
        return value.dump();
    }

    static std::string sanitizeArtifactToken(std::string token) {
        if (token.empty()) {
            return "unknown_bundle";
        }
        for (char& c : token) {
            const bool ok = std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '-';
            if (!ok) c = '_';
        }
        return token;
    }

    static bool hasDiscursiveContent(const Application::DTO::DiscursiveSystemDTO& dto) {
        return !dto.declaredProblems.empty() || !dto.declaredActions.empty() ||
               !dto.allegedMechanisms.empty() || !dto.expectedEffects.empty();
    }

    static bool hasNarrativeContent(const Application::DTO::NarrativeStateDTO& dto) {
        return !dto.axes.empty() || !dto.metadata.empty() || !dto.source.sourceId.empty();
    }

    static bool hasRecommendationContent(const Application::DTO::RecommendationSnapshotDTO& dto) {
        return !dto.recommendationText.empty() || !dto.expectedOutcome.empty() ||
               !dto.contextConditions.empty() || !dto.intendedAction.empty();
    }

    static std::optional<json> loadJsonFile(const std::filesystem::path& filePath) {
        try {
            std::ifstream file(filePath);
            if (!file.is_open()) {
                return std::nullopt;
            }
            json j;
            file >> j;
            return j;
        } catch (...) {
            return std::nullopt;
        }
    }

    static bool isIWPayloadJson(const json& j) {
        return (j.contains("IWBundle") ||
                j.contains("discursiveSystem") ||
                j.contains("narrativeObservations") ||
                j.contains("trajectoryAnalogies") ||
                j.contains("systems") ||
                j.contains("history") ||
                j.contains("snapshots") ||
                j.contains("allegedMechanisms") ||
                j.contains("sourceProfile") ||
                j.contains("baselineAssumptions") ||
                j.contains("discursiveContext") ||
                j.contains("interpretationLayers") ||
                j.contains("temporalWindowReferences"));
    }

    bool isIWBundleDirectory(const std::filesystem::path& dirPath) const {
        if (!std::filesystem::exists(dirPath) || !std::filesystem::is_directory(dirPath)) {
            return false;
        }

        static const std::array<const char*, 10> markers = {
            "IWBundle.json",
            "Manifest.json",
            "DiscursiveSystem.json",
            "NarrativeObservation.json",
            "TrajectoryAnalogies.json",
            "AllegedMechanisms.json",
            "InterpretationLayers.json",
            "DiscursiveContext.json",
            "BaselineAssumptions.json",
            "TemporalWindowReference.json"
        };

        size_t found = 0;
        bool strongMarker = false;
        for (const auto* marker : markers) {
            if (std::filesystem::exists(dirPath / marker)) {
                ++found;
                if (std::string(marker) == "IWBundle.json" || std::string(marker) == "Manifest.json") {
                    strongMarker = true;
                }
            }
        }
        return strongMarker || found >= 3;
    }

    static std::string resolveArtifactId(const std::filesystem::path& bundlePath,
                                         const std::map<std::string, json>& docs) {
        auto fromSourceObject = [](const json& j) -> std::string {
            if (j.contains("source") && j["source"].is_object()) {
                const auto& src = j["source"];
                if (src.contains("artifactId")) return src["artifactId"].get<std::string>();
                if (src.contains("filename")) return src["filename"].get<std::string>();
            }
            return "";
        };

        const auto manifestIt = docs.find("Manifest.json");
        if (manifestIt != docs.end()) {
            const auto& manifest = manifestIt->second;
            if (manifest.contains("artifactId")) {
                return sanitizeArtifactToken(manifest["artifactId"].get<std::string>());
            }
        }

        for (const auto& [_, j] : docs) {
            const std::string candidate = fromSourceObject(j);
            if (!candidate.empty()) {
                return sanitizeArtifactToken(candidate);
            }
        }

        return sanitizeArtifactToken(bundlePath.filename().string());
    }

    static const json* pickPrimaryPayload(const std::map<std::string, json>& docs,
                                          const std::vector<std::string>& precedence) {
        for (const auto& name : precedence) {
            auto it = docs.find(name);
            if (it != docs.end()) {
                return &it->second;
            }
        }
        return nullptr;
    }

    void mergeDiscursiveSupplements(Application::DTO::DiscursiveSystemDTO& dto,
                                    const std::map<std::string, json>& docs) const {
        auto pickSection = [&](const std::string& fileName, const std::string& key) -> std::optional<json> {
            auto fileIt = docs.find(fileName);
            if (fileIt != docs.end() && fileIt->second.contains(key)) {
                return fileIt->second[key];
            }
            auto bundleIt = docs.find("IWBundle.json");
            if (bundleIt != docs.end() && bundleIt->second.contains(key)) {
                return bundleIt->second[key];
            }
            return std::nullopt;
        };

        auto setMetadataIfMissing = [&](const std::string& metadataKey, const std::optional<json>& value) {
            if (!value.has_value()) return;
            auto it = dto.interpretationMetadata.find(metadataKey);
            if (it != dto.interpretationMetadata.end() && !it->second.empty()) return;
            dto.interpretationMetadata[metadataKey] = toMetadataValue(value.value());
        };

        if (dto.allegedMechanisms.empty()) {
            auto alleged = pickSection("AllegedMechanisms.json", "allegedMechanisms");
            if (alleged.has_value()) {
                json patch;
                patch["allegedMechanisms"] = alleged.value();
                auto supplement = Application::Mappers::IW::IWMapper::toDiscursiveSystemDTO(patch);
                dto.allegedMechanisms = supplement.allegedMechanisms;
            }
        }

        setMetadataIfMissing("iw.baselineAssumptions", pickSection("BaselineAssumptions.json", "baselineAssumptions"));
        setMetadataIfMissing("iw.discursiveContext", pickSection("DiscursiveContext.json", "discursiveContext"));
        setMetadataIfMissing("iw.interpretationLayers", pickSection("InterpretationLayers.json", "interpretationLayers"));
        setMetadataIfMissing("iw.temporalWindowReferences", pickSection("TemporalWindowReference.json", "temporalWindowReferences"));
        setMetadataIfMissing("iw.sourceProfile", pickSection("SourceProfile.json", "sourceProfile"));
    }

    bool upsertDiscursiveSystemDTO(const Application::DTO::DiscursiveSystemDTO& dto) {
        auto systems = getDiscursiveSystemDTOs();
        auto it = std::find_if(systems.begin(), systems.end(), [&](const auto& item) {
            return item.id == dto.id;
        });
        if (it != systems.end()) {
            updateDiscursiveSystemDTO(it->id, dto);
            return false;
        }
        registerDiscursiveSystemDTO(dto);
        return true;
    }

    bool upsertNarrativeStateDTO(const Application::DTO::NarrativeStateDTO& dto) {
        auto history = getNarrativeHistoryDTO();

        auto byId = std::find_if(history.begin(), history.end(), [&](const auto& item) {
            return item.id == dto.id;
        });
        if (byId != history.end()) {
            Application::DTO::NarrativeStateDTO updated = dto;
            updated.id = byId->id;
            updateNarrativeStateDTO(byId->id, updated);
            return false;
        }

        auto bySourceAndTime = std::find_if(history.begin(), history.end(), [&](const auto& item) {
            return !item.source.sourceId.empty() &&
                   item.source.sourceId == dto.source.sourceId &&
                   item.temporalContext.label == dto.temporalContext.label;
        });
        if (bySourceAndTime != history.end()) {
            Application::DTO::NarrativeStateDTO updated = dto;
            updated.id = bySourceAndTime->id;
            updateNarrativeStateDTO(bySourceAndTime->id, updated);
            return false;
        }

        registerNarrativeStateDTO(dto);
        return true;
    }

    bool upsertRecommendationSnapshotDTO(const Application::DTO::RecommendationSnapshotDTO& dto) {
        auto trajectory = getRecommendationTrajectoryDTO();
        auto it = std::find_if(trajectory.snapshots.begin(), trajectory.snapshots.end(), [&](const auto& item) {
            return item.id == dto.id;
        });
        if (it != trajectory.snapshots.end()) {
            updateRecommendationSnapshotDTO(it->id, dto);
            return false;
        }
        addRecommendationSnapshotDTO(dto);
        return true;
    }

    void logIWIngestContext(const std::string& artifactId,
                            const std::string& context,
                            size_t mapped,
                            size_t skipped) const {
        std::cout << "[IW Ingest] bundle=" << artifactId
                  << " context=" << context
                  << " mapped=" << mapped
                  << " skipped=" << skipped
                  << std::endl;
    }

    void logIWIngestSummary(const IWIngestSummary& summary) const {
        const size_t totalMapped = summary.discursiveMapped + summary.narrativeMapped + summary.recommendationMapped;
        const size_t totalSkipped = summary.discursiveSkipped + summary.narrativeSkipped + summary.recommendationSkipped;
        const size_t denominator = totalMapped + totalSkipped;
        const double coverage = denominator == 0 ? 0.0 : (100.0 * static_cast<double>(totalMapped) / static_cast<double>(denominator));

        std::cout << "[IW Ingest Summary] bundles=" << summary.bundlesIngested << "/" << summary.bundlesDetected
                  << " standalone=" << summary.standaloneFiles
                  << " discursive=" << summary.discursiveMapped
                  << " narrative=" << summary.narrativeMapped
                  << " recommendation=" << summary.recommendationMapped
                  << " coverage=" << coverage << "%"
                  << std::endl;
    }

    void ingestIWPayload(const std::map<std::string, json>& docs,
                         const std::string& artifactId,
                         IWIngestSummary& summary) {
        const std::string safeArtifact = sanitizeArtifactToken(artifactId);

        // Discursive (DiscursiveSystem -> IWBundle)
        size_t discMapped = 0;
        size_t discSkipped = 0;
        if (const json* discPayload = pickPrimaryPayload(docs, {"DiscursiveSystem.json", "IWBundle.json"})) {
            auto discSystems = Application::Mappers::IW::IWMapper::toDiscursiveSystemDTOs(*discPayload);
            for (size_t i = 0; i < discSystems.size(); ++i) {
                auto dto = discSystems[i];
                if (dto.id.empty()) {
                    dto.id = "DS-IW-" + safeArtifact + "-" + std::to_string(i + 1);
                }
                if (dto.temporalContext.category.empty()) dto.temporalContext.category = "CONTEMPORARY";
                if (dto.temporalContext.label.empty()) dto.temporalContext.label = "IW ingestion";

                mergeDiscursiveSupplements(dto, docs);
                dto.interpretationMetadata["iw.artifactId"] = safeArtifact;

                if (!hasDiscursiveContent(dto)) {
                    ++discSkipped;
                    continue;
                }

                try {
                    upsertDiscursiveSystemDTO(dto);
                    ++discMapped;
                } catch (const std::exception&) {
                    ++discSkipped;
                }
            }
        }
        summary.discursiveMapped += discMapped;
        summary.discursiveSkipped += discSkipped;
        if (discMapped > 0 || discSkipped > 0) {
            logIWIngestContext(safeArtifact, "discursive", discMapped, discSkipped);
        }

        // Narrative (NarrativeObservation -> IWBundle)
        size_t narrMapped = 0;
        size_t narrSkipped = 0;
        if (const json* narrPayload = pickPrimaryPayload(docs, {"NarrativeObservation.json", "IWBundle.json"})) {
            auto narratives = Application::Mappers::IW::IWMapper::toNarrativeStateDTOs(*narrPayload);
            for (size_t i = 0; i < narratives.size(); ++i) {
                auto dto = narratives[i];
                if (dto.id.empty()) {
                    dto.id = "OBS-IW-" + safeArtifact + "-" + std::to_string(i + 1);
                }
                if (dto.source.sourceId.empty()) dto.source.sourceId = safeArtifact;
                if (dto.source.sourceType.empty()) dto.source.sourceType = "SCIENTIFIC_ARTICLE";
                if (dto.temporalContext.category.empty()) dto.temporalContext.category = "CONTEMPORARY";
                if (dto.temporalContext.label.empty()) dto.temporalContext.label = "IW ingestion";

                std::map<std::string, std::string> normalizedMetadata;
                for (const auto& [key, value] : dto.metadata) {
                    if (key.rfind("iw.", 0) == 0) normalizedMetadata[key] = value;
                    else normalizedMetadata["iw." + key] = value;
                }
                normalizedMetadata["iw.artifactId"] = safeArtifact;
                dto.metadata = std::move(normalizedMetadata);

                if (!hasNarrativeContent(dto)) {
                    ++narrSkipped;
                    continue;
                }

                try {
                    upsertNarrativeStateDTO(dto);
                    ++narrMapped;
                } catch (const std::exception&) {
                    ++narrSkipped;
                }
            }
        }
        summary.narrativeMapped += narrMapped;
        summary.narrativeSkipped += narrSkipped;
        if (narrMapped > 0 || narrSkipped > 0) {
            logIWIngestContext(safeArtifact, "narrative", narrMapped, narrSkipped);
        }

        // Recommendation (TrajectoryAnalogies -> IWBundle)
        size_t recMapped = 0;
        size_t recSkipped = 0;
        if (const json* recPayload = pickPrimaryPayload(docs, {"TrajectoryAnalogies.json", "IWBundle.json"})) {
            auto recommendations = Application::Mappers::IW::IWMapper::toRecommendationSnapshotDTOs(*recPayload);
            for (size_t i = 0; i < recommendations.size(); ++i) {
                auto dto = recommendations[i];
                if (dto.id.empty()) {
                    dto.id = "REC-IW-" + safeArtifact + "-" + std::to_string(i + 1);
                }
                if (dto.sourceReference.sourceId.empty()) dto.sourceReference.sourceId = safeArtifact;
                if (dto.sourceReference.sourceType.empty()) dto.sourceReference.sourceType = "DOCUMENT";
                if (dto.temporalContext.category.empty()) dto.temporalContext.category = "CONTEMPORARY";
                if (dto.temporalContext.label.empty()) dto.temporalContext.label = "IW ingestion";

                if (!hasRecommendationContent(dto)) {
                    ++recSkipped;
                    continue;
                }

                try {
                    upsertRecommendationSnapshotDTO(dto);
                    ++recMapped;
                } catch (const std::exception&) {
                    ++recSkipped;
                }
            }
        }
        summary.recommendationMapped += recMapped;
        summary.recommendationSkipped += recSkipped;
        if (recMapped > 0 || recSkipped > 0) {
            logIWIngestContext(safeArtifact, "recommendation", recMapped, recSkipped);
        }
    }

    void ingestIWStandaloneFile(const std::filesystem::path& filePath, IWIngestSummary& summary) {
        auto payload = loadJsonFile(filePath);
        if (!payload.has_value()) {
            return;
        }
        if (!isIWPayloadJson(payload.value())) {
            return;
        }

        std::map<std::string, json> docs;
        docs[filePath.filename().string()] = payload.value();
        const std::string artifactId = resolveArtifactId(filePath.parent_path(), docs);
        ++summary.standaloneFiles;
        ingestIWPayload(docs, artifactId, summary);
    }

    void ingestIWBundleDirectory(const std::filesystem::path& bundleDir, IWIngestSummary& summary) {
        if (!std::filesystem::exists(bundleDir) || !std::filesystem::is_directory(bundleDir)) {
            return;
        }

        std::map<std::string, json> docs;
        for (const auto& entry : std::filesystem::directory_iterator(bundleDir)) {
            if (!entry.is_regular_file() || entry.path().extension() != ".json") {
                continue;
            }
            auto payload = loadJsonFile(entry.path());
            if (payload.has_value()) {
                docs[entry.path().filename().string()] = payload.value();
            }
        }

        if (docs.empty()) {
            return;
        }

        const std::string artifactId = resolveArtifactId(bundleDir, docs);
        ++summary.bundlesIngested;
        ingestIWPayload(docs, artifactId, summary);
    }

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
