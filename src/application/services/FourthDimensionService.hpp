#pragma once

#include "core/domain/fourth_dimension/Trajectory.hpp"
#include "core/domain/fourth_dimension/TrajectoryService.hpp"
#include "core/domain/fourth_dimension/TrajectoryPersistenceService.hpp"
#include "core/domain/fourth_dimension/CoherenceIntensityService.hpp"
#include "core/domain/fourth_dimension/patch_trajectory/PatchTrajectory.hpp"
#include "core/domain/fourth_dimension/patch_trajectory/PatchTrajectoryService.hpp"
#include "core/domain/hydro/HydroGrid.hpp"
#include <optional>

namespace Application::Services {

class FourthDimensionService {
public:
    struct CoherenceResultDTO {
        bool ok = false;
        float mean = -1.0f;
        std::string error;
    };

    static void captureSemanticState(Core::Domain::FourthDimension::Trajectory& trajectory,
                                     const std::vector<int>& semanticState,
                                     const std::vector<bool>& waterMask,
                                     const std::string& metadata) {
        Core::Domain::FourthDimension::TrajectoryService::captureSemanticState(
            trajectory, semanticState, waterMask, metadata
        );
    }

    static void saveTrajectory(Core::Domain::FourthDimension::Trajectory& trajectory,
                               const std::string& dir,
                               const std::string& path) {
        Core::Domain::FourthDimension::TrajectoryPersistenceService::saveTrajectory(trajectory, dir, path);
    }

    static void loadTrajectory(Core::Domain::FourthDimension::Trajectory& trajectory,
                               const std::string& dir,
                               const std::string& path) {
        Core::Domain::FourthDimension::TrajectoryPersistenceService::loadTrajectory(trajectory, dir, path);
    }

    static void loadSliceFromDisk(Core::Domain::FourthDimension::TimeSlice& slice) {
        Core::Domain::FourthDimension::TrajectoryPersistenceService::loadFromDisk(slice);
    }

    static CoherenceResultDTO computeCoherenceMean(
        const Core::Domain::FourthDimension::TimeSlice& sliceA,
        const Core::Domain::FourthDimension::TimeSlice& sliceB,
        const Core::Domain::Hydro::HydroGrid& hydro,
        int radius = 2,
        float sigma = 1.0f,
        float weightType = 0.45f,
        float weightStructure = 0.4f,
        float weightEdge = 0.15f
    ) {
        CoherenceResultDTO out;
        const auto& coverA = sliceA.getEcologicalCoverState();
        const auto& coverB = sliceB.getEcologicalCoverState();
        if (coverA.size() != coverB.size() || coverA.empty()) {
            out.error = "State sizes do not match.";
            return out;
        }

        Core::Domain::FourthDimension::CoherenceIntensityParams params;
        if (hydro.isValid() && (int)hydro.flowAccumulationCells.size() == (int)coverA.size()) {
            params.width = hydro.width;
            params.height = hydro.height;
        }
        params.radius = radius;
        params.sigma = sigma;
        params.weightType = weightType;
        params.weightStructure = weightStructure;
        params.weightEdge = weightEdge;

        auto map = Core::Domain::FourthDimension::CoherenceIntensityService::compare(sliceA, sliceB, params);
        if (map.intensity.empty()) {
            out.error = "Unable to compute map.";
            return out;
        }

        double sum = 0.0;
        for (float v : map.intensity) sum += v;
        out.mean = static_cast<float>(sum / map.intensity.size());
        out.ok = true;
        return out;
    }

    static std::string generatePatchTrajectorySummary(
        const Core::Domain::FourthDimension::PatchTrajectory::PatchTrajectory& trajectory,
        std::function<std::string(int)> nameResolver = nullptr
    ) {
        return Core::Domain::FourthDimension::PatchTrajectory::PatchTrajectoryService::generateLLMSummary(
            trajectory, nameResolver
        );
    }
};

} // namespace Application::Services
