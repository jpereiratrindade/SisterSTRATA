#pragma once

#include "application/dtos/DiscursiveSystemDTO.hpp"
#include "application/dtos/NarrativeDTOs.hpp"
#include "application/dtos/RecommendationTrajectoryDTO.hpp"
#include <nlohmann/json.hpp>
#include <vector>
#include <iostream>
#include <optional>

namespace Application::Mappers::IW {

using json = nlohmann::json;

class IWMapper {
public:
    // Helper: Normalize diverse JSON inputs into a single string
    static std::string normalizeField(const json& element) {
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

    static Application::DTO::DiscursiveSystemDTO toDiscursiveSystemDTO(const json& j) {
        Application::DTO::DiscursiveSystemDTO dto;

        // ID Generation (or use filename if passed, but DTO doesn't know filename here)
        // Leaving ID empty to be handled by caller or generated.
        dto.id = ""; 

        // 1. Basic Fields
        if (j.contains("discursiveSystem")) {
            const auto& ds = j["discursiveSystem"];
            
            if (ds.contains("declaredProblems")) {
                for (const auto& item : ds["declaredProblems"]) {
                    if (item.contains("statement")) {
                        dto.declaredProblems.push_back(normalizeField(item["statement"]));
                    }
                }
            }
            if (ds.contains("declaredActions")) {
                for (const auto& item : ds["declaredActions"]) {
                    if (item.contains("statement")) {
                        dto.declaredActions.push_back(normalizeField(item["statement"]));
                    }
                }
            }
        }
        
        // Check root-level arrays as per example `composicao.pdf.json`
        if (j.contains("allegedMechanisms") && j["allegedMechanisms"].is_array()) {
             for (const auto& item : j["allegedMechanisms"]) {
                 if (item.is_string()) {
                     dto.allegedMechanisms.push_back(normalizeField(item));
                 } else if (item.contains("statement")) {
                     dto.allegedMechanisms.push_back(normalizeField(item["statement"]));
                 }
             }
        }
        
        if (j.contains("discursiveSystem") && j["discursiveSystem"].contains("expectedEffects")) {
             for (const auto& item : j["discursiveSystem"]["expectedEffects"]) {
                 if (item.contains("statement")) {
                     dto.expectedEffects.push_back(normalizeField(item["statement"]));
                 }
             }
        }

        // 2. Source Metadata
        if (j.contains("source")) {
            Application::DTO::SourceReferenceDTO src;
            src.sourceType = "SCIENTIFIC_ARTICLE"; // Default for IW-Consumiveis/scientific
            
            if (j["source"].contains("artifactId")) 
                src.sourceId = normalizeField(j["source"]["artifactId"]);
            else if (j["source"].contains("filename")) 
                src.sourceId = normalizeField(j["source"]["filename"]);
            
            if (j["source"].contains("sourceType")) {
                 src.sourceType = normalizeField(j["source"]["sourceType"]);
            }
            
            dto.sourceReferences.push_back(src);
        }

        // 3. Temporal Context (Try to deduce)
        dto.temporalContext.category = "CONTEMPORARY"; // Default
        dto.temporalContext.label = "Extracted from Scientific Literature";
        
        // Attempt to extract temporal scale from source profile if present
        if (j.contains("sourceProfile") && j["sourceProfile"].contains("temporalScale")) {
             std::string scale = normalizeField(j["sourceProfile"]["temporalScale"]);
             if (!scale.empty()) dto.temporalContext.label += " (" + scale + ")";
        }

        return dto;
    }

    static std::vector<Application::DTO::NarrativeStateDTO> toNarrativeStateDTOs(const json& j) {
        std::vector<Application::DTO::NarrativeStateDTO> dtos;

        if (j.contains("narrativeObservations") && j["narrativeObservations"].is_array()) {
            for (const auto& item : j["narrativeObservations"]) {
                Application::DTO::NarrativeStateDTO dto;
                
                // If the array is empty in example, we can't infer much structure.
                // Assuming standard keys similar to internal DTO if populated.
                
                if (item.contains("observation")) dto.metadata["observation"] = normalizeField(item["observation"]);
                else if (item.contains("evidenceSnippet")) dto.metadata["evidenceSnippet"] = normalizeField(item["evidenceSnippet"]);
                
                if (item.contains("context")) dto.metadata["context"] = normalizeField(item["context"]);
                
                // Map contextuality to metadata
                if (item.contains("contextuality")) {
                    dto.metadata["contextuality"] = normalizeField(item["contextuality"]);
                }

                // Set default intent
                dto.intent.intentType = "Observation";

                dtos.push_back(dto);
            }
        }
        
        return dtos;
    }
    
    // Optional: Extract Recommendation info (trajectory analogies)
    static std::optional<Application::DTO::RecommendationSnapshotDTO> toRecommendationSnapshotDTO(const json& j) {
        if (j.contains("trajectoryAnalogies") && j["trajectoryAnalogies"].is_array() && !j["trajectoryAnalogies"].empty()) {
            auto item = j["trajectoryAnalogies"][0];
            Application::DTO::RecommendationSnapshotDTO dto;
            dto.id = "REC-IW-AUTO";
            
            if (item.contains("analogy")) 
                dto.recommendationText = normalizeField(item["analogy"]);
            
            if (item.contains("justification"))
                dto.expectedOutcome = normalizeField(item["justification"]);

            if (item.contains("scope"))
                dto.contextConditions.push_back(normalizeField(item["scope"]));
            
            // Source
            if (j.contains("source")) {
                 if (j["source"].contains("artifactId")) 
                    dto.sourceReference.sourceId = normalizeField(j["source"]["artifactId"]);
            }
            
            return dto;
        }
        return std::nullopt;
    }
};

} // namespace Application::Mappers::IW
