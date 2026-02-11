#pragma once

#include "application/dtos/DiscursiveSystemDTO.hpp"
#include "application/dtos/NarrativeDTOs.hpp"
#include "application/dtos/RecommendationSnapshotDTO.hpp"
#include "application/dtos/RecommendationTrajectoryDTO.hpp"
#include "application/mappers/ObservationalMappers.hpp"
#include "src/application/mappers/IWMapper.hpp"
#include "src/observational/narrative/aggregates/NarrativeObservationSystem.hpp"
#include "src/observational/discursive/aggregates/DiscursiveSystemRepository.hpp"
#include "src/observational/recommendation/aggregates/RecommendationTrajectory.hpp"

#include <nlohmann/json.hpp>
#include <filesystem>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <array>
#include <optional>

namespace Application::Services {

// Forward declaration
class ProjectPersistenceService;

/**
 * @brief Service responsible for ingesting IdeaWalker (IW) data
 *        into the observational repositories.
 *
 * Handles standalone files, bundle directories, and batch ingestion.
 * Generates ingestion synthesis reports.
 *
 * Extracted from Session.hpp to isolate ingestion logic.
 */
class IWIngestionService {
public:
    struct ArtifactReport {
        std::string artifactId;
        std::string sourceMode; // bundle|standalone
        size_t discursiveMapped = 0;
        size_t discursiveSkipped = 0;
        size_t narrativeMapped = 0;
        size_t narrativeSkipped = 0;
        size_t recommendationMapped = 0;
        size_t recommendationSkipped = 0;
        std::map<std::string, size_t> skipReasons;
    };

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
        std::map<std::string, size_t> skipReasons;
        std::vector<ArtifactReport> artifactReports;

        [[nodiscard]] bool hasAnyActivity() const {
            return bundlesDetected > 0 || bundlesIngested > 0 || standaloneFiles > 0 ||
                   discursiveMapped > 0 || discursiveSkipped > 0 ||
                   narrativeMapped > 0 || narrativeSkipped > 0 ||
                   recommendationMapped > 0 || recommendationSkipped > 0;
        }
    };

    IWIngestionService(
        SisterSTRATA::Observational::Narrative::NarrativeObservationSystem& narrative,
        SisterSTRATA::Observational::Discursive::DiscursiveSystemRepository& discursive,
        SisterSTRATA::Observational::Recommendation::RecommendationTrajectory& recommendation,
        ProjectPersistenceService& persistence,
        const std::filesystem::path& projectRoot);

    void setProjectRoot(const std::filesystem::path& root);

    /**
     * @brief Ingest a single IW file or detect its parent as a bundle.
     */
    void ingestFromIW(const std::string& filepath);

    /**
     * @brief Batch ingest all JSON files from a directory tree.
     */
    void ingestFromIWDirectory(const std::string& dirPath);

    /**
     * @brief Scan project inputs/ directories for ingestion data.
     */
    void scanForIngestion();

    // --- DTO retrieval (used to build context graph in reports) ---
    std::vector<Application::DTO::NarrativeStateDTO> getNarrativeHistoryDTO() const;

private:
    using json = nlohmann::json;

    // --- Helpers ---
    static std::string toMetadataValue(const json& value);
    static std::string sanitizeArtifactToken(std::string token);
    static void incrementSkipReason(std::map<std::string, size_t>& target,
                                    const std::string& context,
                                    const std::string& reason);
    static double computeIngestionCoverage(const IWIngestSummary& summary);
    static std::string nowIsoLike();
    static std::string nowFileToken();
    static bool hasDiscursiveContent(const Application::DTO::DiscursiveSystemDTO& dto);
    static bool hasNarrativeContent(const Application::DTO::NarrativeStateDTO& dto);
    static bool hasRecommendationContent(const Application::DTO::RecommendationSnapshotDTO& dto);
    static std::optional<json> loadJsonFile(const std::filesystem::path& filePath);
    static bool isIWPayloadJson(const json& j);
    bool isIWBundleDirectory(const std::filesystem::path& dirPath) const;
    static std::string resolveArtifactId(const std::filesystem::path& bundlePath,
                                         const std::map<std::string, json>& docs);
    static const json* pickPrimaryPayload(const std::map<std::string, json>& docs,
                                          const std::vector<std::string>& precedence);
    void mergeDiscursiveSupplements(Application::DTO::DiscursiveSystemDTO& dto,
                                   const std::map<std::string, json>& docs) const;

    // --- CRUD upsert wrappers ---
    bool upsertDiscursiveSystemDTO(const Application::DTO::DiscursiveSystemDTO& dto);
    bool upsertNarrativeStateDTO(const Application::DTO::NarrativeStateDTO& dto);
    bool upsertRecommendationSnapshotDTO(const Application::DTO::RecommendationSnapshotDTO& dto);

    // --- Logging ---
    void logIWIngestContext(const std::string& artifactId, const std::string& context,
                            size_t mapped, size_t skipped) const;
    void logIWIngestSummary(const IWIngestSummary& summary) const;

    // --- Report generation ---
    json buildIngestionSynthesisJson(const IWIngestSummary& summary,
                                     const std::string& sourcePath,
                                     const std::string& trigger) const;
    static std::string buildIngestionSynthesisMarkdown(const json& report);
    void writeIngestionSynthesisReport(const IWIngestSummary& summary,
                                       const std::string& sourcePath,
                                       const std::string& trigger) const;

    // --- Core ingestion ---
    void ingestIWPayload(const std::map<std::string, json>& docs,
                         const std::string& artifactId,
                         IWIngestSummary& summary,
                         const std::string& sourceMode);
    void ingestIWStandaloneFile(const std::filesystem::path& filePath, IWIngestSummary& summary);
    void ingestIWBundleDirectory(const std::filesystem::path& bundleDir, IWIngestSummary& summary);

    // --- DTO access helpers ---
    std::vector<Application::DTO::DiscursiveSystemDTO> getDiscursiveSystemDTOs() const;
    Application::DTO::RecommendationTrajectoryDTO getRecommendationTrajectoryDTO() const;

    // References to session-owned repositories
    SisterSTRATA::Observational::Narrative::NarrativeObservationSystem& narrative_;
    SisterSTRATA::Observational::Discursive::DiscursiveSystemRepository& discursive_;
    SisterSTRATA::Observational::Recommendation::RecommendationTrajectory& recommendation_;
    ProjectPersistenceService& persistence_;
    std::filesystem::path projectRoot_;
};

} // namespace Application::Services
