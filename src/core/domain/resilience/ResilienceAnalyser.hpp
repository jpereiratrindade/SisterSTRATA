#pragma once

#include <vector>
#include <string>
#include <cmath>
#include <map>

#include "core/domain/territory/TerritorialTrajectory.hpp"
#include "core/domain/resilience/ResilienceReport.hpp"

namespace Core::Domain::Resilience {

class ResilienceAnalyser {
public:
    /**
     * @brief Analyzes the full trajectory of the territory to assess resilience.
     * 
     * @param trajectory The historical sequence of territory snapshots.
     * @return ResilienceReport A report containing overlap metrics and resilience assessment.
     */
    static ResilienceReport analyzeTrajectory(const Territory::TerritorialTrajectory& trajectory) {
        ResilienceReport report;
        
        if (trajectory.size() < 2) {
            report.finalAssessment = "Insufficient history for resilience analysis (need at least 2 states).";
            return report;
        }

        float totalOverlap = 0.0f;
        int comparisons = 0;

        for (size_t i = 1; i < trajectory.size(); ++i) {
            const auto& t1 = trajectory.getSnapshotAt(i - 1);
            const auto& t2 = trajectory.getSnapshotAt(i);
            
            float overlap = calculateSpatialOverlap(t1, t2);
            totalOverlap += overlap;
            comparisons++;
            
            report.eventLogs.push_back(
                "Transition " + std::to_string(i-1) + "->" + std::to_string(i) + 
                ": Overlap=" + std::to_string(overlap)
            );
        }

        report.meanSpatialOverlap = (comparisons > 0) ? (totalOverlap / comparisons) : 0.0f;
        
        // Simple Threshold Logic (Parametrize later)
        // If overlap > 70% but < 95%, it shows "Adaptive Reconfiguration".
        // If < 50%, "Structural Rupture".
        // If > 99%, "Stasis" (Not necessarily resilience).
        
        if (report.meanSpatialOverlap > 0.99f) {
            report.finalAssessment = "State: STASIS (High stability, low adaptability observed)";
            report.isResilient = true; 
        } else if (report.meanSpatialOverlap > 0.70f) {
             report.finalAssessment = "State: RESILIENT (Coherent reconfiguration maintained)";
             report.isResilient = true;
        } else {
             report.finalAssessment = "State: RUPTURE (Significant loss of spatial structure)";
             report.isResilient = false;
        }

        return report;
    }

    /**
     * @brief Calculates the spatial intersection over union (or just retention) of land use patches.
     * Formula: Area(Patch(t) ∩ Patch(t+1)) / Area(Patch(t))
     * Aggregated for the whole landscape.
     */
    static float calculateSpatialOverlap(const Territory::TerritorySnapshot& t1, const Territory::TerritorySnapshot& t2) {
        // Validation
        const auto& lu1 = t1.getLandUseState();
        const auto& lu2 = t2.getLandUseState();

        if (lu1.size() != lu2.size() || lu1.empty()) return 0.0f;

        size_t matchingCells = 0;
        size_t allocatedCellsT1 = 0;

        for (size_t i = 0; i < lu1.size(); ++i) {
            // Only consider allocated cells for overlap metric (ignore empty space if defined as "")
            if (!lu1[i].empty()) {
                allocatedCellsT1++;
                if (lu1[i] == lu2[i]) {
                    matchingCells++;
                }
            }
        }

        if (allocatedCellsT1 == 0) return 1.0f; // Empty to Empty = Perfect Overlap? Or 0? Let's say 1.0 for consistency.

        return static_cast<float>(matchingCells) / static_cast<float>(allocatedCellsT1);
    }
};

} // namespace Core::Domain::Resilience
