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
                        dto.declaredProblems.push_back(item["statement"].get<std::string>());
                    }
                }
            }
            if (ds.contains("declaredActions")) {
                for (const auto& item : ds["declaredActions"]) {
                    if (item.contains("statement")) {
                        dto.declaredActions.push_back(item["statement"].get<std::string>());
                    }
                }
            }
             if (ds.contains("allegedMechanisms")) { // Note: Might be at root or inside?
                // In example JSON, "allegedMechanisms" is at root, but sometimes might be in discursiveSystem object?
                // Let's check root too in a safe way below.
             }
        }
        
        // Check root-level arrays as per example `composicao.pdf.json`
        if (j.contains("allegedMechanisms") && j["allegedMechanisms"].is_array()) {
             for (const auto& item : j["allegedMechanisms"]) {
                 // Assuming string or object with statement? Example was empty array `[]`.
                 // If it's like others, it's object "statement". If string, handle that.
                 if (item.is_string()) {
                     dto.allegedMechanisms.push_back(item.get<std::string>());
                 } else if (item.contains("statement")) {
                     dto.allegedMechanisms.push_back(item["statement"].get<std::string>());
                 }
             }
        }
        
        if (j.contains("discursiveSystem") && j["discursiveSystem"].contains("expectedEffects")) {
             for (const auto& item : j["discursiveSystem"]["expectedEffects"]) {
                 if (item.contains("statement")) {
                     dto.expectedEffects.push_back(item["statement"].get<std::string>());
                 }
             }
        }

        // 2. Source Metadata
        if (j.contains("source")) {
            Application::DTO::SourceReferenceDTO src;
            src.sourceType = "SCIENTIFIC_ARTICLE"; // Default for IW-Consumiveis/scientific
            if (j["source"].contains("artifactId")) src.sourceId = j["source"]["artifactId"].get<std::string>();
            else if (j["source"].contains("filename")) src.sourceId = j["source"]["filename"].get<std::string>();
            
            // Production Date? Not in example, fallback to empty
            
            dto.sourceReferences.push_back(src);
        }

        // 3. Temporal Context (Try to deduce)
        dto.temporalContext.category = "CONTEMPORARY"; // Default
        dto.temporalContext.label = "Extracted from Scientific Literature";

        return dto;
    }

    static std::vector<Application::DTO::NarrativeStateDTO> toNarrativeStateDTOs(const json& j) {
        std::vector<Application::DTO::NarrativeStateDTO> dtos;

        if (j.contains("narrativeObservations") && j["narrativeObservations"].is_array()) {
            for (const auto& item : j["narrativeObservations"]) {
                Application::DTO::NarrativeStateDTO dto;
                
                // If the array is empty in example, we can't infer much structure.
                // Assuming standard keys similar to internal DTO if populated.
                // For now, if empty, we return empty list.
                
                // Placeholder for future extraction
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
                dto.recommendationText = item["analogy"].get<std::string>();
            
            if (item.contains("justification"))
                dto.expectedOutcome = item["justification"].get<std::string>();

            if (item.contains("scope"))
                dto.contextConditions.push_back(item["scope"].get<std::string>());
            
            // Source
            if (j.contains("source")) {
                 if (j["source"].contains("artifactId")) 
                    dto.sourceReference.sourceId = j["source"]["artifactId"].get<std::string>();
            }
            
            return dto;
        }
        return std::nullopt;
    }
};

} // namespace Application::Mappers::IW
