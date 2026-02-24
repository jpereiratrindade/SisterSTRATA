#include "application/services/IWIngestionService.hpp"
#include "application/services/ProjectPersistenceService.hpp"
#include "application/services/NarrativeContextAnalyzer.hpp"
#include "infrastructure/logging/Logger.hpp"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <algorithm>
#include <cctype>
#include <unordered_set>

namespace Application::Services {

using json = nlohmann::json;

IWIngestionService::IWIngestionService(
    SisterSTRATA::Observational::Narrative::NarrativeObservationSystem& narrative,
    SisterSTRATA::Observational::Discursive::DiscursiveSystemRepository& discursive,
    SisterSTRATA::Observational::Recommendation::RecommendationTrajectory& recommendation,
    ProjectPersistenceService& persistence,
    const std::filesystem::path& projectRoot)
    : narrative_(narrative),
      discursive_(discursive),
      recommendation_(recommendation),
      persistence_(persistence),
      projectRoot_(projectRoot) {}

void IWIngestionService::setProjectRoot(const std::filesystem::path& root) {
    projectRoot_ = root;
}

// ===========================================================================
// Public API
// ===========================================================================

void IWIngestionService::ingestFromIW(const std::string& filepath) {
    LOG_INFO("IWIngestion", std::string("Ingesting from IW: ") + filepath);
    std::filesystem::path filePath(filepath);
    if (!std::filesystem::exists(filePath) || !std::filesystem::is_regular_file(filePath)) {
        LOG_ERROR("IWIngestion", std::string("Invalid IW file: ") + filepath);
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
    writeIngestionSynthesisReport(summary, filePath.parent_path().string(), "ingest_single_file");
}

void IWIngestionService::ingestFromIWDirectory(const std::string& dirPath) {
    LOG_INFO("IWIngestion", std::string("Batch Ingestion from Directory: ") + dirPath);
    std::filesystem::path root(dirPath);
    if (!std::filesystem::exists(root) || !std::filesystem::is_directory(root)) {
        LOG_ERROR("IWIngestion", std::string("Invalid directory for ingestion: ") + dirPath);
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

    if (bundleDirs.empty()) {
        std::sort(standaloneFiles.begin(), standaloneFiles.end());
        for (const auto& file : standaloneFiles) {
            ingestIWStandaloneFile(file, summary);
        }
    }

    logIWIngestSummary(summary);
    writeIngestionSynthesisReport(summary, root.string(), "ingest_directory");
    LOG_INFO("IWIngestion", "Batch Ingestion Complete.");
}

void IWIngestionService::scanForIngestion() {
    LOG_INFO("IWIngestion", "Scanning project for inputs...");

    std::filesystem::path inputsDir = projectRoot_ / "inputs";
    if (!std::filesystem::exists(inputsDir)) {
        try {
            std::filesystem::create_directories(inputsDir / "narratives");
            std::filesystem::create_directories(inputsDir / "discursive");
            LOG_INFO("IWIngestion", std::string("Created input directories at ") + inputsDir.string());
        } catch (...) {}
        return;
    }

    IWIngestSummary summary;
    auto scanDir = [&](const std::filesystem::path& dir) {
         if (!std::filesystem::exists(dir)) return;
         for (const auto& entry : std::filesystem::directory_iterator(dir)) {
             if (entry.is_regular_file() && entry.path().extension() == ".json") {
                 ingestIWStandaloneFile(entry.path(), summary);
             }
         }
    };

    scanDir(inputsDir / "narratives");
    scanDir(inputsDir / "discursive");
    if (summary.hasAnyActivity()) {
        logIWIngestSummary(summary);
        writeIngestionSynthesisReport(summary, inputsDir.string(), "scan_project_inputs");
    }
}

std::vector<Application::DTO::NarrativeStateDTO> IWIngestionService::getNarrativeHistoryDTO() const {
    std::vector<Application::DTO::NarrativeStateDTO> dtos;
    for (const auto& state : narrative_.getHistory()) {
        dtos.push_back(Application::Mappers::Narrative::toDTO(state));
    }
    return dtos;
}

// ===========================================================================
// Static Helpers
// ===========================================================================

std::string IWIngestionService::toMetadataValue(const json& value) {
    if (value.is_string()) return value.get<std::string>();
    if (value.is_number_integer()) return std::to_string(value.get<long long>());
    if (value.is_number_unsigned()) return std::to_string(value.get<unsigned long long>());
    if (value.is_number_float()) return std::to_string(value.get<double>());
    if (value.is_boolean()) return value.get<bool>() ? "true" : "false";
    if (value.is_null()) return "";
    return value.dump();
}

std::string IWIngestionService::sanitizeArtifactToken(std::string token) {
    if (token.empty()) {
        return "unknown_bundle";
    }
    for (char& c : token) {
        const bool ok = std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '-';
        if (!ok) c = '_';
    }
    return token;
}

void IWIngestionService::incrementSkipReason(std::map<std::string, size_t>& target,
                                              const std::string& context,
                                              const std::string& reason) {
    target[context + "." + reason] += 1;
}

double IWIngestionService::computeIngestionCoverage(const IWIngestSummary& summary) {
    const size_t totalMapped = summary.discursiveMapped + summary.narrativeMapped + summary.recommendationMapped;
    const size_t totalSkipped = summary.discursiveSkipped + summary.narrativeSkipped + summary.recommendationSkipped;
    const size_t denominator = totalMapped + totalSkipped;
    return denominator == 0 ? 0.0 : (100.0 * static_cast<double>(totalMapped) / static_cast<double>(denominator));
}

std::string IWIngestionService::nowIsoLike() {
    auto now = std::chrono::system_clock::now();
    std::time_t tt = std::chrono::system_clock::to_time_t(now);
    std::tm tm {};
#if defined(_WIN32)
    localtime_s(&tm, &tt);
#else
    localtime_r(&tt, &tm);
#endif
    std::ostringstream out;
    out << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return out.str();
}

std::string IWIngestionService::nowFileToken() {
    auto now = std::chrono::system_clock::now();
    std::time_t tt = std::chrono::system_clock::to_time_t(now);
    std::tm tm {};
#if defined(_WIN32)
    localtime_s(&tm, &tt);
#else
    localtime_r(&tt, &tm);
#endif
    std::ostringstream out;
    out << std::put_time(&tm, "%Y%m%d_%H%M%S");
    return out.str();
}

bool IWIngestionService::hasDiscursiveContent(const Application::DTO::DiscursiveSystemDTO& dto) {
    return !dto.declaredProblems.empty() || !dto.declaredActions.empty() ||
           !dto.allegedMechanisms.empty() || !dto.expectedEffects.empty();
}

bool IWIngestionService::hasNarrativeContent(const Application::DTO::NarrativeStateDTO& dto) {
    return !dto.axes.empty() || !dto.metadata.empty() || !dto.source.sourceId.empty();
}

bool IWIngestionService::hasRecommendationContent(const Application::DTO::RecommendationSnapshotDTO& dto) {
    return !dto.recommendationText.empty() || !dto.expectedOutcome.empty() ||
           !dto.contextConditions.empty() || !dto.intendedAction.empty();
}

std::optional<json> IWIngestionService::loadJsonFile(const std::filesystem::path& filePath) {
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

bool IWIngestionService::isIWPayloadJson(const json& j) {
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

bool IWIngestionService::isIWBundleDirectory(const std::filesystem::path& dirPath) const {
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

std::string IWIngestionService::resolveArtifactId(const std::filesystem::path& bundlePath,
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

const json* IWIngestionService::pickPrimaryPayload(const std::map<std::string, json>& docs,
                                                    const std::vector<std::string>& precedence) {
    for (const auto& name : precedence) {
        auto it = docs.find(name);
        if (it != docs.end()) {
            return &it->second;
        }
    }
    return nullptr;
}

const json* pickPayloadByKeys(const std::map<std::string, json>& docs,
                              const std::vector<std::string>& keys) {
    for (const auto& [_, doc] : docs) {
        for (const auto& key : keys) {
            if (doc.contains(key)) {
                return &doc;
            }
        }
    }
    return nullptr;
}

void IWIngestionService::mergeDiscursiveSupplements(Application::DTO::DiscursiveSystemDTO& dto,
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

// ===========================================================================
// CRUD Upsert Wrappers
// ===========================================================================

std::vector<Application::DTO::DiscursiveSystemDTO> IWIngestionService::getDiscursiveSystemDTOs() const {
    std::vector<Application::DTO::DiscursiveSystemDTO> dtos;
    for (const auto& system : discursive_.getSystems()) {
        dtos.push_back(Application::Mappers::Discursive::toDTO(system));
    }
    return dtos;
}

Application::DTO::RecommendationTrajectoryDTO IWIngestionService::getRecommendationTrajectoryDTO() const {
    return Application::Mappers::Recommendation::toDTO(recommendation_);
}

bool IWIngestionService::upsertDiscursiveSystemDTO(const Application::DTO::DiscursiveSystemDTO& dto) {
    auto systems = getDiscursiveSystemDTOs();
    auto it = std::find_if(systems.begin(), systems.end(), [&](const auto& item) {
        return item.id == dto.id;
    });
    if (it != systems.end()) {
        discursive_.updateSystem(it->id, Application::Mappers::Discursive::toDomain(dto));
        persistence_.autoSaveDiscursive();
        return false;
    }
    discursive_.registerSystem(Application::Mappers::Discursive::toDomain(dto));
    persistence_.autoSaveDiscursive();
    return true;
}

bool IWIngestionService::upsertNarrativeStateDTO(const Application::DTO::NarrativeStateDTO& dto) {
    auto history = getNarrativeHistoryDTO();

    auto byId = std::find_if(history.begin(), history.end(), [&](const auto& item) {
        return item.id == dto.id;
    });
    if (byId != history.end()) {
        Application::DTO::NarrativeStateDTO updated = dto;
        updated.id = byId->id;
        narrative_.updateObservation(byId->id, Application::Mappers::Narrative::toDomain(updated));
        persistence_.autoSaveNarrative();
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
        narrative_.updateObservation(bySourceAndTime->id, Application::Mappers::Narrative::toDomain(updated));
        persistence_.autoSaveNarrative();
        return false;
    }

    narrative_.registerObservation(Application::Mappers::Narrative::toDomain(dto));
    persistence_.autoSaveNarrative();
    return true;
}

bool IWIngestionService::upsertRecommendationSnapshotDTO(const Application::DTO::RecommendationSnapshotDTO& dto) {
    auto trajectory = getRecommendationTrajectoryDTO();
    auto it = std::find_if(trajectory.snapshots.begin(), trajectory.snapshots.end(), [&](const auto& item) {
        return item.id == dto.id;
    });
    if (it != trajectory.snapshots.end()) {
        recommendation_.updateSnapshot(it->id, Application::Mappers::Recommendation::toDomain(dto));
        persistence_.autoSaveRecommendation();
        return false;
    }
    if (recommendation_.getId().empty()) {
        recommendation_ = SisterSTRATA::Observational::Recommendation::RecommendationTrajectory(
            "REC-TRAJECTORY-1", {}
        );
    }
    recommendation_.addSnapshot(Application::Mappers::Recommendation::toDomain(dto));
    persistence_.autoSaveRecommendation();
    return true;
}

// ===========================================================================
// Logging
// ===========================================================================

void IWIngestionService::logIWIngestContext(const std::string& artifactId,
                                            const std::string& context,
                                            size_t mapped,
                                            size_t skipped) const {
    LOG_INFO("IWIngest", "bundle=" + artifactId + " context=" + context + " mapped=" + std::to_string(mapped) + " skipped=" + std::to_string(skipped));
}

void IWIngestionService::logIWIngestSummary(const IWIngestSummary& summary) const {
    const double coverage = computeIngestionCoverage(summary);

    LOG_INFO("IWIngest", "bundles=" + std::to_string(summary.bundlesIngested) + "/" + std::to_string(summary.bundlesDetected)
        + " standalone=" + std::to_string(summary.standaloneFiles)
        + " discursive=" + std::to_string(summary.discursiveMapped)
        + " narrative=" + std::to_string(summary.narrativeMapped)
        + " recommendation=" + std::to_string(summary.recommendationMapped)
        + " coverage=" + std::to_string(coverage) + "%");
}

// ===========================================================================
// Report Generation
// ===========================================================================

json IWIngestionService::buildIngestionSynthesisJson(const IWIngestSummary& summary,
                                                     const std::string& sourcePath,
                                                     const std::string& trigger) const {
    json j;
    j["schemaVersion"] = 1;
    j["reportType"] = "IngestionSynthesisReport";
    j["generatedAt"] = nowIsoLike();
    j["projectRoot"] = projectRoot_.string();
    j["sourcePath"] = sourcePath;
    j["trigger"] = trigger;
    j["pipelineVersion"] = "iw_ingest_v1";

    j["epistemicStatus"] = {
        {"type", "observational_synthesis"},
        {"allowsResilienceInference", false},
        {"requiresSpatialTemporalData", true},
        {"notes", "Report is descriptive. No causal or prescriptive inference is performed."}
    };

    j["summary"] = {
        {"bundlesDetected", summary.bundlesDetected},
        {"bundlesIngested", summary.bundlesIngested},
        {"standaloneFiles", summary.standaloneFiles},
        {"coveragePercent", computeIngestionCoverage(summary)}
    };

    j["contexts"] = {
        {"discursive", {{"mapped", summary.discursiveMapped}, {"skipped", summary.discursiveSkipped}}},
        {"narrative", {{"mapped", summary.narrativeMapped}, {"skipped", summary.narrativeSkipped}}},
        {"recommendation", {{"mapped", summary.recommendationMapped}, {"skipped", summary.recommendationSkipped}}}
    };

    j["skipReasons"] = summary.skipReasons;
    auto artifacts = summary.artifactReports;
    std::sort(artifacts.begin(), artifacts.end(), [](const auto& a, const auto& b) {
        return a.artifactId < b.artifactId;
    });

    j["artifacts"] = json::array();
    for (const auto& artifact : artifacts) {
        j["artifacts"].push_back({
            {"artifactId", artifact.artifactId},
            {"sourceMode", artifact.sourceMode},
            {"discursive", {{"mapped", artifact.discursiveMapped}, {"skipped", artifact.discursiveSkipped}}},
            {"narrative", {{"mapped", artifact.narrativeMapped}, {"skipped", artifact.narrativeSkipped}}},
            {"recommendation", {{"mapped", artifact.recommendationMapped}, {"skipped", artifact.recommendationSkipped}}},
            {"skipReasons", artifact.skipReasons}
        });
    }

    // Use the NarrativeContextAnalyzer for the graph
    j["narrativeContextGraph"] = NarrativeContextAnalyzer::buildContextGraph(getNarrativeHistoryDTO());
    return j;
}

std::string IWIngestionService::buildIngestionSynthesisMarkdown(const json& report) {
    std::ostringstream md;
    md << "# Ingestion Synthesis Report\n\n";
    md << "- Generated At: " << report.value("generatedAt", "unknown") << "\n";
    md << "- Project Root: " << report.value("projectRoot", "unknown") << "\n";
    md << "- Source Path: " << report.value("sourcePath", "unknown") << "\n";
    md << "- Trigger: " << report.value("trigger", "unknown") << "\n\n";

    const auto summary = report.value("summary", json::object());
    md << "## Summary\n\n";
    md << "- Bundles Detected: " << summary.value("bundlesDetected", 0) << "\n";
    md << "- Bundles Ingested: " << summary.value("bundlesIngested", 0) << "\n";
    md << "- Standalone Files: " << summary.value("standaloneFiles", 0) << "\n";
    md << "- Coverage (%): " << summary.value("coveragePercent", 0.0) << "\n\n";

    const auto contexts = report.value("contexts", json::object());
    const auto contextRow = [&](const char* key) {
        const auto c = contexts.value(key, json::object());
        md << "| " << key << " | " << c.value("mapped", 0) << " | " << c.value("skipped", 0) << " |\n";
    };
    md << "## Context Metrics\n\n";
    md << "| Context | Mapped | Skipped |\n";
    md << "| --- | ---: | ---: |\n";
    contextRow("discursive");
    contextRow("narrative");
    contextRow("recommendation");
    md << "\n";

    const auto artifacts = report.value("artifacts", json::array());
    md << "## Artifacts\n\n";
    md << "| Artifact ID | Source Mode | Discursive | Narrative | Recommendation |\n";
    md << "| --- | --- | ---: | ---: | ---: |\n";
    for (const auto& item : artifacts) {
        const auto disc = item.value("discursive", json::object());
        const auto narr = item.value("narrative", json::object());
        const auto rec = item.value("recommendation", json::object());
        md << "| " << item.value("artifactId", "unknown")
           << " | " << item.value("sourceMode", "unknown")
           << " | " << disc.value("mapped", 0)
           << " | " << narr.value("mapped", 0)
           << " | " << rec.value("mapped", 0)
           << " |\n";
    }
    md << "\n";

    md << "## Epistemic Status\n\n";
    const auto status = report.value("epistemicStatus", json::object());
    md << "- Type: " << status.value("type", "unknown") << "\n";
    md << "- allowsResilienceInference: " << (status.value("allowsResilienceInference", false) ? "true" : "false") << "\n";
    md << "- requiresSpatialTemporalData: " << (status.value("requiresSpatialTemporalData", false) ? "true" : "false") << "\n";
    md << "- Notes: " << status.value("notes", "") << "\n";

    const auto graph = report.value("narrativeContextGraph", json::object());
    md << "\n## Narrative Context Graph\n\n";
    md << "- distanceType: " << graph.value("distanceType", "unknown") << "\n";
    md << "- causalInterpretationAllowed: " << (graph.value("causalInterpretationAllowed", true) ? "true" : "false") << "\n";
    md << "- nodes: " << graph.value("nodes", json::array()).size() << "\n";
    md << "- edges: " << graph.value("edges", json::array()).size() << "\n";
    return md.str();
}

void IWIngestionService::writeIngestionSynthesisReport(const IWIngestSummary& summary,
                                                        const std::string& sourcePath,
                                                        const std::string& trigger) const {
    if (!summary.hasAnyActivity()) {
        return;
    }

    try {
        const json report = buildIngestionSynthesisJson(summary, sourcePath, trigger);
        const std::string markdown = buildIngestionSynthesisMarkdown(report);
        const std::string stamp = nowFileToken();

        const std::filesystem::path reportDir = projectRoot_ / "reports" / "ingestion";
        std::filesystem::create_directories(reportDir);

        const auto latestJsonPath = reportDir / "IngestionSynthesisReport.latest.json";
        const auto latestMdPath = reportDir / "IngestionSynthesisReport.latest.md";
        const auto stampedJsonPath = reportDir / ("IngestionSynthesisReport_" + stamp + ".json");
        const auto stampedMdPath = reportDir / ("IngestionSynthesisReport_" + stamp + ".md");

        {
            std::ofstream out(latestJsonPath);
            if (out.is_open()) out << report.dump(2);
        }
        {
            std::ofstream out(stampedJsonPath);
            if (out.is_open()) out << report.dump(2);
        }
        {
            std::ofstream out(latestMdPath);
            if (out.is_open()) out << markdown;
        }
        {
            std::ofstream out(stampedMdPath);
            if (out.is_open()) out << markdown;
        }

        LOG_INFO("IWIngest", std::string("Report written: ") + latestJsonPath.string());
    } catch (const std::exception& e) {
        LOG_ERROR("IWIngest", std::string("Report failed: ") + e.what());
    }
}

// ===========================================================================
// Core Ingestion
// ===========================================================================

void IWIngestionService::ingestIWPayload(const std::map<std::string, json>& docs,
                                          const std::string& artifactId,
                                          IWIngestSummary& summary,
                                          const std::string& sourceMode) {
    const std::string safeArtifact = sanitizeArtifactToken(artifactId);
    ArtifactReport artifactReport;
    artifactReport.artifactId = safeArtifact;
    artifactReport.sourceMode = sourceMode;

    // Discursive
    size_t discMapped = 0;
    size_t discSkipped = 0;
    const json* discPayload = pickPrimaryPayload(docs, {"DiscursiveSystem.json", "IWBundle.json"});
    if (!discPayload) {
        discPayload = pickPayloadByKeys(docs, {"systems", "discursiveSystem", "allegedMechanisms"});
    }
    if (discPayload) {
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
                incrementSkipReason(artifactReport.skipReasons, "discursive", "empty_or_invalid_content");
                continue;
            }

            try {
                upsertDiscursiveSystemDTO(dto);
                ++discMapped;
            } catch (const std::exception&) {
                ++discSkipped;
                incrementSkipReason(artifactReport.skipReasons, "discursive", "persistence_error");
            }
        }
    }
    summary.discursiveMapped += discMapped;
    summary.discursiveSkipped += discSkipped;
    artifactReport.discursiveMapped = discMapped;
    artifactReport.discursiveSkipped = discSkipped;
    if (discMapped > 0 || discSkipped > 0) {
        logIWIngestContext(safeArtifact, "discursive", discMapped, discSkipped);
    }

    // Narrative
    size_t narrMapped = 0;
    size_t narrSkipped = 0;
    const json* narrPayload = pickPrimaryPayload(docs, {"NarrativeObservation.json", "IWBundle.json"});
    if (!narrPayload) {
        narrPayload = pickPayloadByKeys(docs, {"history", "narrativeObservations"});
    }
    if (narrPayload) {
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
                incrementSkipReason(artifactReport.skipReasons, "narrative", "empty_or_invalid_content");
                continue;
            }

            try {
                upsertNarrativeStateDTO(dto);
                ++narrMapped;
            } catch (const std::exception&) {
                ++narrSkipped;
                incrementSkipReason(artifactReport.skipReasons, "narrative", "persistence_error");
            }
        }
    }
    summary.narrativeMapped += narrMapped;
    summary.narrativeSkipped += narrSkipped;
    artifactReport.narrativeMapped = narrMapped;
    artifactReport.narrativeSkipped = narrSkipped;
    if (narrMapped > 0 || narrSkipped > 0) {
        logIWIngestContext(safeArtifact, "narrative", narrMapped, narrSkipped);
    }

    // Recommendation
    size_t recMapped = 0;
    size_t recSkipped = 0;
    const json* recPayload = pickPrimaryPayload(docs, {"TrajectoryAnalogies.json", "IWBundle.json"});
    if (!recPayload) {
        recPayload = pickPayloadByKeys(docs, {"snapshots", "trajectoryAnalogies"});
    }
    if (recPayload) {
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
                incrementSkipReason(artifactReport.skipReasons, "recommendation", "empty_or_invalid_content");
                continue;
            }

            try {
                upsertRecommendationSnapshotDTO(dto);
                ++recMapped;
            } catch (const std::exception&) {
                ++recSkipped;
                incrementSkipReason(artifactReport.skipReasons, "recommendation", "persistence_error");
            }
        }
    }
    summary.recommendationMapped += recMapped;
    summary.recommendationSkipped += recSkipped;
    artifactReport.recommendationMapped = recMapped;
    artifactReport.recommendationSkipped = recSkipped;
    if (recMapped > 0 || recSkipped > 0) {
        logIWIngestContext(safeArtifact, "recommendation", recMapped, recSkipped);
    }

    for (const auto& [reason, count] : artifactReport.skipReasons) {
        summary.skipReasons[reason] += count;
    }
    summary.artifactReports.push_back(std::move(artifactReport));
}

void IWIngestionService::ingestIWStandaloneFile(const std::filesystem::path& filePath, IWIngestSummary& summary) {
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
    ingestIWPayload(docs, artifactId, summary, "standalone");
}

void IWIngestionService::ingestIWBundleDirectory(const std::filesystem::path& bundleDir, IWIngestSummary& summary) {
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
    ingestIWPayload(docs, artifactId, summary, "bundle");
}

} // namespace Application::Services
