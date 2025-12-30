#pragma once

#include <vector>
#include <memory>
#include <string>

#include "core/domain/hydro/HydroGrid.hpp"
#include "core/domain/soils/SoilSystem.hpp"
#include "core/domain/shared/value_objects/CoherenceScore.hpp"

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
     * @brief Updates the calculated coherence score.
     * Should be called by the TerritorialCoherenceService after analysis.
     */
    void setCoherenceScore(Shared::ValueObjects::CoherenceScore score) {
        coherenceScore_ = std::move(score);
    }

private:
    TerritoryID id_;
    int width_;
    int height_;

    // Orchestrated State Layers
    // Note: These represent the "Snapshot" of the territory state.
    Hydro::HydroGrid hydroLayer_;
    std::vector<Soils::SiBCSClassification> soilLayer_;

    // Result Metadata
    Shared::ValueObjects::CoherenceScore coherenceScore_{0.0f, "Uninitialized"};
};

} // namespace Core::Domain::Territory
