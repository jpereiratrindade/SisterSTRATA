#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace Application::DTO::Cognitive {

/**
 * @brief DTO representing a semantic packet for AI interpretation.
 * Focuses on "Rendered Text" to maintain the epistemological boundary.
 */
struct ContextBundleDTO {
    std::string bundleId;
    std::string intent;                     // "theme_analysis", "discursive_draft", etc.
    std::vector<std::string> narratives;     // Human-readable narrative projections
    std::vector<std::string> discursive;     // Human-readable discursive system projections
    std::string recommendation;             // Human-readable recommendation projection
    std::string trajectorySummary;          // Textual summary of the Fourth Dimension
    std::string trajectoryImpactProfile;    // New: Analytical Impact Profile

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(ContextBundleDTO, 
        bundleId, intent, narratives, discursive, recommendation, trajectorySummary, trajectoryImpactProfile)
};

} // namespace Application::DTO::Cognitive
