#pragma once

#include "application/dtos/DiscursiveSystemDTO.hpp"
#include "application/dtos/NarrativeDTOs.hpp"
#include "application/dtos/RecommendationTrajectoryDTO.hpp"
#include <nlohmann/json.hpp>
#include <vector>
#include <iostream>
#include <optional>
#include <map>
#include <string>

namespace Application::Mappers::IW {

using json = nlohmann::json;

class IWMapper {
public:
    static constexpr const char* DEFAULT_TIME_CATEGORY = "CONTEMPORARY";

private:
    static std::string metadataValue(const json& value) {
        if (value.is_string() || value.is_number() || value.is_boolean() || value.is_null()) {
            return normalizeField(value);
        }
        return value.dump();
    }

    static std::string toIWMetadataKey(const std::string& key) {
        if (key.rfind("iw.", 0) == 0) {
            return key;
        }
        return "iw." + key;
    }

    static std::string parseNarrativeSourceType(const json& value) {
        if (value.is_number_integer()) {
            switch (value.get<int>()) {
                case 0: return "INTERVIEW";
                case 1: return "TECHNICAL_REPORT";
                case 2: return "HISTORICAL_RECORD";
                case 3: return "SCIENTIFIC_ARTICLE";
                case 4: return "INSTITUTIONAL_DOCUMENT";
                case 5: return "MEDIA_ARTICLE";
                case 6: return "FIELD_NOTE";
                default: return "OTHER";
            }
        }
        return normalizeField(value);
    }

    static std::string parseDiscursiveSourceType(const json& value) {
        if (value.is_number_integer()) {
            switch (value.get<int>()) {
                case 0: return "INTERVIEW";
                case 1: return "DOCUMENT";
                case 2: return "QUESTIONNAIRE";
                case 3: return "TECHNICAL_BULLETIN";
                case 4: return "REPORT";
                default: return "OTHER";
            }
        }
        return normalizeField(value);
    }

    static std::string parseNarrativeTemporalCategory(const json& value) {
        if (value.is_number_integer()) {
            switch (value.get<int>()) {
                case 0: return "ANCESTRAL";
                case 1: return "PAST";
                case 2: return "RECENT_PAST";
                case 3: return "CONTEMPORARY";
                case 4: return "FUTURE_VISION";
                case 5: return "TIMELESS";
                default: return "INDETERMINATE";
            }
        }
        return normalizeField(value);
    }

    static std::string parseDiscursiveTemporalCategory(const json& value) {
        return parseNarrativeTemporalCategory(value);
    }

    static std::string parseNarrativeIntent(const json& value) {
        if (value.is_number_integer()) {
            switch (value.get<int>()) {
                case 0: return "DESCRIPTIVE_RECORD";
                case 1: return "EXPLORATORY_HYPOTHESIS";
                case 2: return "CONTEXTUALIZATION";
                case 3: return "METHODOLOGICAL_NOTE";
                default: return "DESCRIPTIVE_RECORD";
            }
        }
        return normalizeField(value);
    }

    static std::string parseAbstractionLevel(const json& value) {
        if (value.is_number_integer()) {
            switch (value.get<int>()) {
                case 0: return "LOCAL";
                case 1: return "REGIONAL";
                case 2: return "INSTITUTIONAL";
                case 3: return "GLOBAL";
                default: return "LOCAL";
            }
        }
        return normalizeField(value);
    }

    static std::vector<std::string> extractStringArray(const json& node,
                                                       const std::vector<std::string>& preferredKeys) {
        std::vector<std::string> values;
        if (!node.is_array()) {
            return values;
        }

        for (const auto& item : node) {
            if (item.is_string() || item.is_number() || item.is_boolean()) {
                std::string value = normalizeField(item);
                if (!value.empty()) values.push_back(value);
                continue;
            }

            if (!item.is_object()) {
                continue;
            }

            std::string value;
            for (const auto& key : preferredKeys) {
                if (item.contains(key)) {
                    value = normalizeField(item[key]);
                    break;
                }
            }
            if (value.empty()) {
                if (item.contains("statement")) {
                    value = normalizeField(item["statement"]);
                } else if (item.contains("value")) {
                    value = normalizeField(item["value"]);
                }
            }

            if (!value.empty()) {
                values.push_back(value);
            }
        }
        return values;
    }

    static void fillSourceFromRoot(const json& root,
                                   Application::DTO::SourceReferenceDTO& source,
                                   const std::string& fallbackType) {
        if (root.contains("source") && root["source"].is_object()) {
            const auto& src = root["source"];
            if (source.sourceId.empty()) {
                if (src.contains("artifactId")) source.sourceId = normalizeField(src["artifactId"]);
                else if (src.contains("filename")) source.sourceId = normalizeField(src["filename"]);
            }
            if (source.sourceType.empty()) {
                if (src.contains("sourceType")) source.sourceType = normalizeField(src["sourceType"]);
                else source.sourceType = fallbackType;
            }
            if (source.productionDate.empty() && src.contains("ingestedAt")) {
                source.productionDate = normalizeField(src["ingestedAt"]);
            }
            if (!source.author.has_value() && src.contains("author")) {
                source.author = normalizeField(src["author"]);
            }
        }
        if (source.sourceType.empty()) {
            source.sourceType = fallbackType;
        }
    }

    static Application::DTO::DiscursiveSystemDTO parseDiscursiveSystemNode(const json& system,
                                                                            const json& root) {
        Application::DTO::DiscursiveSystemDTO dto;

        if (system.contains("id")) {
            dto.id = normalizeField(system["id"]);
        }

        if (system.contains("declaredProblems")) {
            dto.declaredProblems = extractStringArray(system["declaredProblems"], {"statement", "problem"});
        }
        if (system.contains("declaredActions")) {
            dto.declaredActions = extractStringArray(system["declaredActions"], {"statement", "action"});
        }
        if (system.contains("allegedMechanisms")) {
            dto.allegedMechanisms = extractStringArray(system["allegedMechanisms"], {"statement", "mechanism"});
        }
        if (system.contains("expectedEffects")) {
            dto.expectedEffects = extractStringArray(system["expectedEffects"], {"statement", "effect"});
        }

        if (system.contains("sourceReferences") && system["sourceReferences"].is_array()) {
            for (const auto& srcNode : system["sourceReferences"]) {
                if (!srcNode.is_object()) continue;
                Application::DTO::SourceReferenceDTO source;
                if (srcNode.contains("type")) source.sourceType = parseDiscursiveSourceType(srcNode["type"]);
                else if (srcNode.contains("sourceType")) source.sourceType = normalizeField(srcNode["sourceType"]);
                if (srcNode.contains("sourceId")) source.sourceId = normalizeField(srcNode["sourceId"]);
                if (srcNode.contains("productionDate")) source.productionDate = normalizeField(srcNode["productionDate"]);
                if (srcNode.contains("author")) source.author = normalizeField(srcNode["author"]);
                fillSourceFromRoot(root, source, "DOCUMENT");
                dto.sourceReferences.push_back(source);
            }
        }

        if (system.contains("temporalContext") && system["temporalContext"].is_object()) {
            const auto& temporal = system["temporalContext"];
            if (temporal.contains("category")) {
                dto.temporalContext.category = parseDiscursiveTemporalCategory(temporal["category"]);
            }
            if (temporal.contains("label")) {
                dto.temporalContext.label = normalizeField(temporal["label"]);
            }
        }
        if (dto.temporalContext.category.empty()) {
            dto.temporalContext.category = DEFAULT_TIME_CATEGORY;
        }
        if (dto.temporalContext.label.empty()) {
            dto.temporalContext.label = "Extracted from IW";
        }

        if (system.contains("interpretationMetadata") && system["interpretationMetadata"].is_object()) {
            for (auto it = system["interpretationMetadata"].begin(); it != system["interpretationMetadata"].end(); ++it) {
                dto.interpretationMetadata[it.key()] = metadataValue(it.value());
            }
        }

        if (dto.sourceReferences.empty()) {
            Application::DTO::SourceReferenceDTO source;
            fillSourceFromRoot(root, source, "DOCUMENT");
            if (!source.sourceId.empty()) {
                dto.sourceReferences.push_back(source);
            }
        }

        return dto;
    }

public:
    // Helper: Normalize diverse JSON inputs into a single string
    static std::string normalizeField(const json& element) {
        if (element.is_null()) {
            return "";
        }

        if (element.is_string()) {
            std::string val = element.get<std::string>();
            // Handle pipe-separated values (e.g. "theoretical|simulation")
            // Strategy: Take the first one, or "mixed" if it seems appropriate,
            // but for now let's just take the first one to be safe and canonical.
            if (val.find('|') != std::string::npos) {
                std::cout << "[IWMapper] Warning: Normalizing pipe-separated value: " << val << " -> ";
                val = val.substr(0, val.find('|'));
                std::cout << val << std::endl;
            }
            return val;
        }

        if (element.is_number_integer()) {
            return std::to_string(element.get<long long>());
        }
        if (element.is_number_unsigned()) {
            return std::to_string(element.get<unsigned long long>());
        }
        if (element.is_number_float()) {
            return std::to_string(element.get<double>());
        }
        if (element.is_boolean()) {
            return element.get<bool>() ? "true" : "false";
        }

        // Handle vector of candidates (future proofing)
        // Expected format: [ { "value": "A", "confidence": 0.9 }, ... ] OR simple strings
        if (element.is_array() && !element.empty()) {
            // If array of strings, take first
            if (element[0].is_string()) {
                return normalizeField(element[0]);
            }
            // If array of objects with "value"
            if (element[0].is_object() && element[0].contains("value")) {
                // Ideally pick highest confidence, but for now picker first is stable
                return normalizeField(element[0]["value"]);
            }
        }

        // Handle object with "value" directly
        if (element.is_object() && element.contains("value")) {
             return normalizeField(element["value"]);
        }

        return "";
    }

    static std::vector<Application::DTO::DiscursiveSystemDTO> toDiscursiveSystemDTOs(const json& j) {
        std::vector<Application::DTO::DiscursiveSystemDTO> dtos;

        if (j.contains("systems") && j["systems"].is_array()) {
            for (const auto& system : j["systems"]) {
                if (!system.is_object()) continue;
                dtos.push_back(parseDiscursiveSystemNode(system, j));
            }
            return dtos;
        }

        const json* rootSystem = nullptr;
        if (j.contains("discursiveSystem") && j["discursiveSystem"].is_object()) {
            rootSystem = &j["discursiveSystem"];
        } else if (j.contains("declaredProblems") || j.contains("declaredActions") ||
                   j.contains("allegedMechanisms") || j.contains("expectedEffects")) {
            rootSystem = &j;
        }

        if (rootSystem) {
            auto dto = parseDiscursiveSystemNode(*rootSystem, j);
            if (dto.allegedMechanisms.empty() && j.contains("allegedMechanisms")) {
                dto.allegedMechanisms = extractStringArray(j["allegedMechanisms"], {"statement", "mechanism"});
            }
            dtos.push_back(dto);
        }

        return dtos;
    }

    static Application::DTO::DiscursiveSystemDTO toDiscursiveSystemDTO(const json& j) {
        auto dtos = toDiscursiveSystemDTOs(j);
        if (dtos.empty()) {
            return Application::DTO::DiscursiveSystemDTO{};
        }
        return dtos.front();
    }

    static std::vector<Application::DTO::NarrativeStateDTO> toNarrativeStateDTOs(const json& j) {
        std::vector<Application::DTO::NarrativeStateDTO> dtos;

        if (j.contains("history") && j["history"].is_array()) {
            for (const auto& item : j["history"]) {
                if (!item.is_object()) continue;

                Application::DTO::NarrativeStateDTO dto;
                if (item.contains("id")) {
                    dto.id = normalizeField(item["id"]);
                }

                if (item.contains("source") && item["source"].is_object()) {
                    const auto& srcNode = item["source"];
                    if (srcNode.contains("type")) dto.source.sourceType = parseNarrativeSourceType(srcNode["type"]);
                    else if (srcNode.contains("sourceType")) dto.source.sourceType = normalizeField(srcNode["sourceType"]);
                    if (srcNode.contains("sourceId")) dto.source.sourceId = normalizeField(srcNode["sourceId"]);
                    if (srcNode.contains("productionDate")) dto.source.productionDate = normalizeField(srcNode["productionDate"]);
                    if (srcNode.contains("author")) dto.source.author = normalizeField(srcNode["author"]);
                }
                fillSourceFromRoot(j, dto.source, "SCIENTIFIC_ARTICLE");

                if (item.contains("temporalContext") && item["temporalContext"].is_object()) {
                    const auto& temporal = item["temporalContext"];
                    if (temporal.contains("category")) {
                        dto.temporalContext.category = parseNarrativeTemporalCategory(temporal["category"]);
                    }
                    if (temporal.contains("label")) {
                        dto.temporalContext.label = normalizeField(temporal["label"]);
                    }
                }
                if (dto.temporalContext.category.empty()) dto.temporalContext.category = DEFAULT_TIME_CATEGORY;

                if (item.contains("intent") && item["intent"].is_object()) {
                    const auto& intent = item["intent"];
                    if (intent.contains("type")) {
                        dto.intent.intentType = parseNarrativeIntent(intent["type"]);
                    }
                }
                if (dto.intent.intentType.empty()) dto.intent.intentType = "DESCRIPTIVE_RECORD";

                if (item.contains("axes") && item["axes"].is_array()) {
                    for (const auto& axisNode : item["axes"]) {
                        if (!axisNode.is_object()) continue;
                        Application::DTO::SemanticAxisDTO axis;
                        if (axisNode.contains("label")) axis.label = normalizeField(axisNode["label"]);
                        if (axisNode.contains("description")) axis.description = normalizeField(axisNode["description"]);
                        else axis.description = axis.label;
                        if (axisNode.contains("level")) axis.abstractionLevel = parseAbstractionLevel(axisNode["level"]);
                        else if (axisNode.contains("abstractionLevel")) axis.abstractionLevel = normalizeField(axisNode["abstractionLevel"]);
                        else axis.abstractionLevel = "LOCAL";
                        if (!axis.label.empty() || !axis.description.empty()) {
                            dto.axes.push_back(axis);
                        }
                    }
                }

                if (item.contains("metadata") && item["metadata"].is_object()) {
                    for (auto it = item["metadata"].begin(); it != item["metadata"].end(); ++it) {
                        dto.metadata[toIWMetadataKey(it.key())] = metadataValue(it.value());
                    }
                }

                if (dto.temporalContext.label.empty()) {
                    auto it = dto.metadata.find("iw.context");
                    if (it != dto.metadata.end()) {
                        dto.temporalContext.label = it->second;
                    }
                }
                if (dto.temporalContext.label.empty()) {
                    dto.temporalContext.label = "IW narrative ingestion";
                }
                if (dto.metadata.find("iw.artifactId") == dto.metadata.end() &&
                    j.contains("source") && j["source"].contains("artifactId")) {
                    dto.metadata["iw.artifactId"] = normalizeField(j["source"]["artifactId"]);
                }
                if (dto.metadata.find("iw.observation") != dto.metadata.end() && dto.axes.empty()) {
                    const std::string observation = dto.metadata["iw.observation"];
                    dto.axes.push_back(Application::DTO::SemanticAxisDTO{"extracted_theme", observation, "LOCAL"});
                }
                dtos.push_back(dto);
            }
            return dtos;
        }

        if (j.contains("narrativeObservations") && j["narrativeObservations"].is_array()) {
            for (const auto& item : j["narrativeObservations"]) {
                if (!item.is_object()) continue;
                Application::DTO::NarrativeStateDTO dto;

                if (item.contains("id")) {
                    dto.id = normalizeField(item["id"]);
                }

                fillSourceFromRoot(j, dto.source, "SCIENTIFIC_ARTICLE");
                dto.temporalContext.category = DEFAULT_TIME_CATEGORY;
                dto.temporalContext.label = item.contains("context") ? normalizeField(item["context"]) : "IW narrative ingestion";
                dto.intent.intentType = "DESCRIPTIVE_RECORD";

                for (auto it = item.begin(); it != item.end(); ++it) {
                    dto.metadata[toIWMetadataKey(it.key())] = metadataValue(it.value());
                }

                if (item.contains("observation")) {
                    std::string observation = normalizeField(item["observation"]);
                    dto.axes.push_back(Application::DTO::SemanticAxisDTO{
                        "extracted_theme",
                        observation,
                        "LOCAL"
                    });
                } else if (item.contains("evidenceSnippet")) {
                    dto.axes.push_back(Application::DTO::SemanticAxisDTO{
                        "extracted_theme",
                        normalizeField(item["evidenceSnippet"]),
                        "LOCAL"
                    });
                }

                if (dto.metadata.find("iw.artifactId") == dto.metadata.end() &&
                    j.contains("source") && j["source"].contains("artifactId")) {
                    dto.metadata["iw.artifactId"] = normalizeField(j["source"]["artifactId"]);
                }

                dtos.push_back(dto);
            }
        }

        return dtos;
    }

    static std::vector<Application::DTO::RecommendationSnapshotDTO> toRecommendationSnapshotDTOs(const json& j) {
        std::vector<Application::DTO::RecommendationSnapshotDTO> dtos;

        auto applySourceAndDefaults = [&](Application::DTO::RecommendationSnapshotDTO& dto) {
            fillSourceFromRoot(j, dto.sourceReference, "DOCUMENT");
            if (dto.temporalContext.category.empty()) {
                dto.temporalContext.category = DEFAULT_TIME_CATEGORY;
            }
            if (dto.temporalContext.label.empty()) {
                if (!dto.sourceReference.productionDate.empty()) {
                    dto.temporalContext.label = dto.sourceReference.productionDate;
                } else {
                    dto.temporalContext.label = "IW recommendation ingestion";
                }
            }
        };

        if (j.contains("snapshots") && j["snapshots"].is_array()) {
            for (const auto& item : j["snapshots"]) {
                if (!item.is_object()) continue;
                Application::DTO::RecommendationSnapshotDTO dto;
                if (item.contains("id")) dto.id = normalizeField(item["id"]);
                if (item.contains("recommendationText")) dto.recommendationText = normalizeField(item["recommendationText"]);
                if (item.contains("expectedOutcome")) dto.expectedOutcome = normalizeField(item["expectedOutcome"]);
                if (item.contains("intendedAction")) dto.intendedAction = normalizeField(item["intendedAction"]);
                if (item.contains("contextConditions") && item["contextConditions"].is_array()) {
                    for (const auto& cond : item["contextConditions"]) {
                        dto.contextConditions.push_back(normalizeField(cond));
                    }
                }
                if (item.contains("sourceReference") && item["sourceReference"].is_object()) {
                    const auto& src = item["sourceReference"];
                    if (src.contains("sourceType")) dto.sourceReference.sourceType = normalizeField(src["sourceType"]);
                    if (src.contains("sourceId")) dto.sourceReference.sourceId = normalizeField(src["sourceId"]);
                    if (src.contains("productionDate")) dto.sourceReference.productionDate = normalizeField(src["productionDate"]);
                    if (src.contains("author")) dto.sourceReference.author = normalizeField(src["author"]);
                }
                if (item.contains("temporalContext") && item["temporalContext"].is_object()) {
                    const auto& temporal = item["temporalContext"];
                    if (temporal.contains("category")) dto.temporalContext.category = parseNarrativeTemporalCategory(temporal["category"]);
                    if (temporal.contains("label")) dto.temporalContext.label = normalizeField(temporal["label"]);
                }
                applySourceAndDefaults(dto);
                dtos.push_back(dto);
            }
            return dtos;
        }

        if (j.contains("trajectoryAnalogies") && j["trajectoryAnalogies"].is_array()) {
            for (const auto& item : j["trajectoryAnalogies"]) {
                if (!item.is_object()) continue;

                Application::DTO::RecommendationSnapshotDTO dto;
                if (item.contains("id")) {
                    dto.id = normalizeField(item["id"]);
                }
                if (item.contains("analogy")) {
                    dto.recommendationText = normalizeField(item["analogy"]);
                }
                if (item.contains("justification")) {
                    dto.expectedOutcome = normalizeField(item["justification"]);
                }
                if (item.contains("scope")) {
                    dto.contextConditions.push_back(normalizeField(item["scope"]));
                }
                if (item.contains("action")) {
                    dto.intendedAction = normalizeField(item["action"]);
                } else if (item.contains("intendedAction")) {
                    dto.intendedAction = normalizeField(item["intendedAction"]);
                }
                if (item.contains("source") && item["source"].is_object()) {
                    const auto& src = item["source"];
                    if (src.contains("sourceType")) dto.sourceReference.sourceType = normalizeField(src["sourceType"]);
                    if (src.contains("sourceId")) dto.sourceReference.sourceId = normalizeField(src["sourceId"]);
                    if (src.contains("artifactId") && dto.sourceReference.sourceId.empty()) dto.sourceReference.sourceId = normalizeField(src["artifactId"]);
                    if (src.contains("productionDate")) dto.sourceReference.productionDate = normalizeField(src["productionDate"]);
                    if (src.contains("author")) dto.sourceReference.author = normalizeField(src["author"]);
                }
                applySourceAndDefaults(dto);
                dtos.push_back(dto);
            }
        }

        return dtos;
    }

    // Optional: Extract Recommendation info (trajectory analogies)
    static std::optional<Application::DTO::RecommendationSnapshotDTO> toRecommendationSnapshotDTO(const json& j) {
        auto dtos = toRecommendationSnapshotDTOs(j);
        if (!dtos.empty()) {
            return dtos.front();
        }
        return std::nullopt;
    }
};

} // namespace Application::Mappers::IW
