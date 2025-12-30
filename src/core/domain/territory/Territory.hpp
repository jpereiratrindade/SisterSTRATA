#pragma once

#include <vector>
#include <memory>
#include <string>

#include "core/domain/hydro/HydroGrid.hpp"
#include "core/domain/soils/SoilSystem.hpp"

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

    int getWidth() const { return width_; }
    int getHeight() const { return height_; }

    // --- Orchestration Accessors ---
    
    /**
     * @brief Provides read-access to the Hydrology component of the territory.
     */
    const Hydro::HydroGrid& getHydroLayer() const { return hydroLayer_; }
    
    /**
     * @brief Allows the engine to update the hydro layer state.
     */
    void updateHydroLayer(Hydro::HydroGrid grid) {
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

private:
    TerritoryID id_;
    int width_;
    int height_;

    // Orchestrated State Layers
    Hydro::HydroGrid hydroLayer_;
    std::vector<Soils::SiBCSClassification> soilLayer_;
    std::vector<float> slopeLayer_; 
};

} // namespace Core::Domain::Territory

} // namespace Core::Domain::Territory
