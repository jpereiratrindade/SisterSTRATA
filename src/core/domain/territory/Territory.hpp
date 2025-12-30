#pragma once

#include <vector>
#include <memory>
#include <string>

#include "core/domain/hydro/HydroGrid.hpp"
#include "core/domain/soils/SoilSystem.hpp"
#include "core/domain/shared/value_objects/CoherenceScore.hpp"
#include "core/domain/territory/TerritorialTrajectory.hpp"

namespace Core::Domain::Territory {

/**
 * @brief Aggregate Root representing the simulated territorial space.
 * 
 * The Territory is the central orchestrator of the STRATA domain. 
 * It does not "own" the raw data of every grid (performance optimization),
 * but it manages the identity, lifecycle, and consistency state of the 
 * bio-physical and spatial layers.
 * 
 * @invariant All managed grids must share the same dimensions.
 * @invariant CoherenceScore must reflect the current state of the territory components.
 */
class Territory {
public:
    using TerritoryID = std::string;

    /**
     * @brief Constructs a new Territory.
     * @param id Unique identifier for this territory instance.
     * @param width Spatial width in grid cells.
     * @param height Spatial height in grid cells.
     */
    Territory(TerritoryID id, int width, int height) 
        : id_(std::move(id)), width_(width), height_(height) {}

    /**
     * @brief Gets the unique identity of the territory.
     */
    const TerritoryID& getId() const { return id_; }

    /**
     * @brief Gets the unified coherence score of the current territorial state.
     * The score represents how well the active components (Land Use, BioPhysical attributes) align.
     */
    const Shared::ValueObjects::CoherenceScore& getCoherenceScore() const { return coherenceScore_; }

    int getWidth() const { return width_; }
    int getHeight() const { return height_; }

    // --- Orchestration Accessors ---
    
    /**
     * @brief Provides read-access to the Hydrology component of the territory.
     */
    const Hydro::HydroGrid& getHydroLayer() const { return hydroLayer_; }
    
    /**
     * @brief Allows the engine to update the hydro layer state.
     * In a pure DDD approach, this might be handled via a Domain Event or Service,
     * but for performance in this simulation context, we allow controlled injection.
     */
    void updateHydroLayer(Hydro::HydroGrid grid) {
        // Validation could go here (dimensions check)
        hydroLayer_ = std::move(grid);
    }

    /**
     * @brief Provides read-access to the Soil classification layer.
     */
    const std::vector<Soils::SiBCSClassification>& getSoilLayer() const { return soilLayer_; }

    /**
     * @brief Updates the soil layer state.
     */
    void updateSoilLayer(std::vector<Soils::SiBCSClassification> soils) {
        soilLayer_ = std::move(soils);
    }

    /**
     * @brief Provides read-access to the Slope map (in degrees).
     */
    const std::vector<float>& getSlopeLayer() const { return slopeLayer_; }

    /**
     * @brief Updates the slope layer state.
     */
    void updateSlopeLayer(std::vector<float> slopes) {
        slopeLayer_ = std::move(slopes);
    }

    /**
     * @brief Updates the calculated coherence score.
     * Should be called by the TerritorialCoherenceService after analysis.
     */
    void setCoherenceScore(Shared::ValueObjects::CoherenceScore score) {
        coherenceScore_ = std::move(score);
    }

    /**
     * @brief Commits the current mutable state as an immutable snapshot in the trajectory.
     * @param hypothesisId Context of the change.
     * @param landUse The emergent land use configuration resulting from the hypothesis.
     */
    void commitState(const std::string& hypothesisId, const std::vector<std::string>& landUse) {
        // Create snapshot from current fields
        TerritorySnapshot snapshot(
            trajectory_.size(), // Event Index
            std::chrono::system_clock::now(),
            hypothesisId,
            hydroLayer_,   // Copy
            soilLayer_,    // Copy
            slopeLayer_,   // Copy
            landUse        // Copy
        );
        trajectory_.addSnapshot(std::move(snapshot));
    }

    /**
     * @brief Access to the historical trajectory for resilience analysis.
     */
    const TerritorialTrajectory& getTrajectory() const { return trajectory_; }

private:
    TerritoryID id_;
    int width_;
    int height_;

    // Orchestrated State Layers
    // Note: These represent the "Snapshot" of the territory state.
    Hydro::HydroGrid hydroLayer_;
    std::vector<Soils::SiBCSClassification> soilLayer_;
    std::vector<float> slopeLayer_; // New: Slope map in degrees

    // Result Metadata
    Shared::ValueObjects::CoherenceScore coherenceScore_{0.0f, "Uninitialized"};

    // Timeline
    TerritorialTrajectory trajectory_;
};

} // namespace Core::Domain::Territory
