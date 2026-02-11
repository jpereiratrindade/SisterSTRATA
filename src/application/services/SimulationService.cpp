#include "application/services/SimulationService.hpp"
#include "infrastructure/logging/Logger.hpp"

namespace Application::Services {

void SimulationService::simulateCondition(SimulationType type,
                                          Core::Domain::Workspace& workspace,
                                          Core::Domain::FourthDimension::Trajectory& trajectory) {
    auto* world = workspace.getWorld().get();

    // Auto-create world if missing (Mock for Simulation)
    if (!world) {
        workspace.createWorld("Simulation Environment", 100, 100);
        world = workspace.getWorld().get();
        LOG_WARN("SimulationService", "Terrain Generation currently unavailable in Headless/Session context.");
    }

    if (!world) return;

    trajectory.clear();

    int w = world->getResolution().width;
    int h = world->getResolution().height;
    size_t size = w * h;

    // Helper to generate a slice
    auto addSlice = [&](int ordinal, const std::vector<int>& cover, const std::string& meta) {
         std::vector<bool> water(size, false);
         trajectory.addTimeSlice(Core::Domain::FourthDimension::TimeSlice(
             ordinal, ordinal, cover, water, meta,
             Core::Domain::FourthDimension::ClassificationType::SemanticCode
         ));
    };

    std::vector<int> baseline(size, 1); // Full Forest

    if (type == SimulationType::Stability) {
        addSlice(1, baseline, "Baseline Year 1");
        addSlice(2, baseline, "Baseline Year 5"); // Identical
    }
    else if (type == SimulationType::Fragmentation) {
        addSlice(1, baseline, "Baseline (Intact)");

        // Checkerboard pattern (High Fragmentation)
        std::vector<int> fragmented(size);
        for(int y=0; y<h; ++y) {
            for(int x=0; x<w; ++x) {
                fragmented[y*w + x] = ((x/10 + y/10) % 2 == 0) ? 1 : -1;
            }
        }
        addSlice(2, fragmented, "Simulated Fragmentation");
    }
    else if (type == SimulationType::Deforestation) {
        addSlice(1, baseline, "Baseline (Intact)");

        // Massive loss (80% gone)
        std::vector<int> deforested(size, -1);
        // Keep small patch in corner
        for(size_t i=0; i<size/10; ++i) deforested[i] = 1;

        addSlice(2, deforested, "Simulated Deforestation");
    }
}

} // namespace Application::Services
