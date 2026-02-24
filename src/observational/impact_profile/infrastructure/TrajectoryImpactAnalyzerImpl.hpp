#pragma once

#include "observational/impact_profile/domain/services/TrajectoryImpactAnalyzer.hpp"
#include "core/domain/spatial_pattern/PatchAnalysis.hpp"
#include <cmath>
#include <numeric>

namespace SisterSTRATA::Observational::ImpactProfile::Infrastructure {

using namespace SisterSTRATA::Observational::ImpactProfile::Domain;
using namespace Core::Domain::FourthDimension;
using namespace Core::Domain::SpatialPattern;

class TrajectoryImpactAnalyzerImpl : public ITrajectoryImpactAnalyzer {
public:
    // Dimensions required for spatial reconstruction from flat vectors
    TrajectoryImpactAnalyzerImpl(int width, int height) 
        : width_(width), height_(height) {}

    TrajectoryImpactProfile analyze(
        const Trajectory& observed,
        const Trajectory& reference,
        const ReferenceFrame& context
    ) const override {
        
        // 1. Calculate baselines (from Reference)
        // For simplicity, we compare the LATEST state of both trajectories.
        // A more complex version would compare the entire time-series.
        const TimeSlice* refSlice = reference.getLatest();
        const TimeSlice* obsSlice = observed.getLatest();

        if (!refSlice || !obsSlice) {
            // Handle edge case: empty trajectories
            // Returning a "Neutral" or "Empty" profile
             return TrajectoryImpactProfile(
                "ERR-EMPTY",
                context.referenceId, // using ref ID as placeholder
                context,
                StructuralDeviation(0,0,0, "Insufficient Data"),
                TemporalDeviationPattern(DeviationTrend::Erratic, 0, 0, "No data points")
            );
        }

        auto refMetrics = analyzeSlice(*refSlice);
        auto obsMetrics = analyzeSlice(*obsSlice);

        // 2. Compute Structural Deviation (Deltas)
        // Delta = Observed - Reference
        // Positive Fragmentation Delta = More fragmented than reference (Bad?)
        // Positive Coherence Delta = More coherent than reference
        
        // Using "Mean Shape Index" as proxy for Fragmentation (higher = more complex/fragmented shapes)
        // Using "Mean RCC" (Related Circumscribing Circle) as proxy for Coherence (closer to 1 = more compact/coherent)
        
        double fragDelta = obsMetrics.meanShapeIndex - refMetrics.meanShapeIndex;
        double coherenceDelta = obsMetrics.meanRcc - refMetrics.meanRcc; 
        
        // Area trend delta? We'd need time series. calculating simplified area diff for now.
        double areaDelta = obsMetrics.areaTotal - refMetrics.areaTotal;

        std::string semanticTag = "Stable";
        if (std::abs(fragDelta) > 0.5) semanticTag = fragDelta > 0 ? "High Fragmentation" : "Simplified Structure";
        if (std::abs(coherenceDelta) > 0.2) semanticTag += ", Structural Shift";

        StructuralDeviation structDev(coherenceDelta, fragDelta, areaDelta, semanticTag);

        // 3. Compute Temporal Deviation Pattern
        // Requires looking at history. For MVP, we check just the last 2 steps if available
        // to determine immediate trend.
        DeviationTrend trend = DeviationTrend::Parallel;
        std::string trendDesc = "Steady State";

        // Logic placeholder for trend (MVP)
        if (std::abs(areaDelta) > 100.0) {
             trend = DeviationTrend::Divergent;
             trendDesc = "Significant Divergence in Biomass/Area";
        }

        TemporalDeviationPattern tempPattern(trend, 0.0, 0.0, trendDesc);

        // 4. Construct Aggregate Root
        // ID Generation: usually UUID, here simple string const
        std::string profileId = "PROF-" + std::to_string(std::time(nullptr));

        return TrajectoryImpactProfile(
            profileId,
            "OBS-CURRENT", // Ideally passed or derived
            context,
            structDev,
            tempPattern
        );
    }

private:
    int width_;
    int height_;

    SummaryMetrics analyzeSlice(const TimeSlice& slice) const {
        GridData grid;
        grid.width = width_;
        grid.height = height_;
        
        // Convert int cover to double for analysis
        const auto& cover = slice.getEcologicalCoverState();
        grid.values.resize(cover.size());
        for(size_t i=0; i<cover.size(); ++i) {
            grid.values[i] = static_cast<double>(cover[i]);
        }

        AnalysisConfig cfg;
        cfg.threshold = 0.5; // Assuming >0 is "something"
        cfg.byClass = false; // Analyze as binary landscape (Occupied vs Empty) for structural metrics

        AnalysisResult result = AnalyzeGrid(grid, cfg);
        return result.summary;
    }
};

} // namespace SisterSTRATA::Observational::ImpactProfile::Infrastructure
