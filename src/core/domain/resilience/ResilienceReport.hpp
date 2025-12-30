#pragma once

#include <vector>
#include <string>

namespace Core::Domain::Resilience {

struct ResilienceReport {
    bool isResilient = false;
    float meanSpatialOverlap = 0.0f;
    float corePersistenceIndex = 0.0f;
    float trajectoryCoherence = 0.0f;
    
    std::vector<std::string> eventLogs;
    std::string finalAssessment;
};

} // namespace Core::Domain::Resilience
