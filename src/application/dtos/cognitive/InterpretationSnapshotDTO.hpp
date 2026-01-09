#pragma once

#include <string>
#include <nlohmann/json.hpp>

namespace Application::DTO::Cognitive {

/**
 * @brief DTO for storing AI interpretations as "Epistemic Memory".
 */
struct InterpretationSnapshotDTO {
    std::string snapshotId;
    std::string createdAt;                  // ISO 8601 or similar
    std::string intent;                     // Which mode was used
    std::string inputContextSummary;        // High-level summary of what was sent
    std::string aiOutput;                   // The raw (or lightly cleaned) AI response
    std::string promptVersion;              // Which version of the Canonical Prompt was used
    std::string sourceBundleId;             // Reference to the original context bundle

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(InterpretationSnapshotDTO,
        snapshotId, createdAt, intent, inputContextSummary, aiOutput, promptVersion, sourceBundleId)
};

} // namespace Application::DTO::Cognitive
