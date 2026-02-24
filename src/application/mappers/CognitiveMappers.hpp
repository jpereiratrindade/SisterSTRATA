#pragma once

#include "application/dtos/cognitive/ContextBundleDTO.hpp"
#include "application/dtos/DiscursiveSystemDTO.hpp"
#include "application/dtos/NarrativeDTOs.hpp"
#include "application/dtos/RecommendationSnapshotDTO.hpp"
#include <sstream>
#include <vector>
#include <iomanip>

namespace Application::Mappers::Cognitive {

/**
 * @brief Renders a NarrativeStateDTO into a structured, readable paragraph.
 */
inline std::string renderNarrative(const Application::DTO::NarrativeStateDTO& dto) {
    std::stringstream ss;
    ss << "--- OBSERVATION [" << dto.id << "] ---\n";
    ss << "Source: " << dto.source.sourceId << " (" << dto.source.productionDate << ")\n";
    ss << "Temporal Context: " << dto.temporalContext.label << " [" << dto.temporalContext.category << "]\n";
    ss << "Interpretation Intent: " << dto.intent.intentType << "\n";
    ss << "Dominant Themes: ";
    for (size_t i = 0; i < dto.axes.size(); ++i) {
        ss << dto.axes[i].label << (i < dto.axes.size() - 1 ? ", " : "");
    }
    ss << "\n";
    
    if (!dto.metadata.empty()) {
        ss << "Interpretation Metadata:\n";
        for (const auto& [key, value] : dto.metadata) {
            ss << "  - " << key << ": " << value << "\n";
        }
    }
    return ss.str();
}

/**
 * @brief Renders a DiscursiveSystemDTO into a descriptive text packet.
 */
inline std::string renderDiscursiveSystem(const Application::DTO::DiscursiveSystemDTO& dto) {
    std::stringstream ss;
    ss << "### DISCURSIVE SYSTEM [" << dto.id << "]\n";
    ss << "Declared context for time: " << dto.temporalContext.label << "\n";
    if (!dto.sourceReferences.empty()) {
        ss << "Source Refs: ";
        for (size_t i = 0; i < dto.sourceReferences.size(); ++i) {
            ss << dto.sourceReferences[i].sourceId;
            if (i + 1 < dto.sourceReferences.size()) ss << ", ";
        }
        ss << "\n";
    }
    
    if (!dto.declaredProblems.empty()) {
        ss << "DECLARED PROBLEMS:\n";
        for (const auto& p : dto.declaredProblems) ss << "  * " << p << "\n";
    }
    
    if (!dto.declaredActions.empty()) {
        ss << "DECLARED ACTIONS (SOLUTIONS):\n";
        for (const auto& a : dto.declaredActions) ss << "  * " << a << "\n";
    }
    
    if (!dto.allegedMechanisms.empty()) {
        ss << "ALLEGED MECHANISMS (CAUSALITY):\n";
        for (const auto& m : dto.allegedMechanisms) ss << "  * " << m << "\n";
    }
    
    if (!dto.expectedEffects.empty()) {
        ss << "EXPECTED EFFECTS (OUTCOMES):\n";
        for (const auto& e : dto.expectedEffects) ss << "  * " << e << "\n";
    }
    
    return ss.str();
}

/**
 * @brief Renders a RecommendationSnapshotDTO into a prescriptive timeline entry.
 */
inline std::string renderRecommendation(const Application::DTO::RecommendationSnapshotDTO& dto) {
    std::stringstream ss;
    ss << "=> RECOMMENDATION SNAPSHOT [" << dto.id << "]\n";
    ss << "Source: " << dto.sourceReference.sourceId << " (" << dto.sourceReference.productionDate << ")\n";
    ss << "Time Label: " << dto.temporalContext.label << "\n";
    ss << "Recommendation Text: \"" << dto.recommendationText << "\"\n";
    ss << "Intended Action: " << dto.intendedAction << "\n";
    ss << "Expected Outcome: " << dto.expectedOutcome << "\n";
    
    if (!dto.contextConditions.empty()) {
        ss << "Active Conditions: ";
        for (size_t i = 0; i < dto.contextConditions.size(); ++i) {
            ss << dto.contextConditions[i] << (i < dto.contextConditions.size() - 1 ? ", " : "");
        }
        ss << "\n";
    }
    return ss.str();
}

/**
 * @brief Aggregates domain DTOs into a ContextBundleDTO.
 */
inline Application::DTO::Cognitive::ContextBundleDTO createBundle(
    const std::string& intent,
    const std::vector<Application::DTO::NarrativeStateDTO>& narratives,
    const std::vector<Application::DTO::DiscursiveSystemDTO>& discursive = {},
    const Application::DTO::RecommendationTrajectoryDTO* recommendations = nullptr,
    const std::string& trajectorySummary = ""
) {
    Application::DTO::Cognitive::ContextBundleDTO bundle;
    bundle.bundleId = "BUNDLE-" + std::to_string(std::time(nullptr));
    bundle.intent = intent;
    bundle.trajectorySummary = trajectorySummary;

    for (const auto& n : narratives) {
        bundle.narratives.push_back(renderNarrative(n));
    }
    
    for (const auto& d : discursive) {
        bundle.discursive.push_back(renderDiscursiveSystem(d));
    }
    
    if (recommendations) {
        std::stringstream ss;
        ss << "TRAJECTORY ID: " << recommendations->id << "\n";
        for (const auto& s : recommendations->snapshots) {
            ss << renderRecommendation(s) << "\n";
        }
        bundle.recommendation = ss.str();
    }

    return bundle;
}

} // namespace Application::Mappers::Cognitive
