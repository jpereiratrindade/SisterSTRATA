#pragma once

#include "core/domain/vegetation/VegetationOriginal.hpp"
#include <vector>
#include <string>

namespace Core::Domain::Vegetation {

/**
 * @brief Represents a set of vegetation hypotheses grouped by a single ID.
 * This represents a "Vector of States" (Water, Forest, Campo) for a specific scenario.
 * It encapsulates multiple VegetationOriginal components that together define an ecological state.
 */
class EcologicalScenario {
public:
    /**
     * @brief Constructor for EcologicalScenario.
     * @param id The unique identifier for this scenario (e.g., "Hypothesis_01").
     */
    EcologicalScenario(std::string id) : id_(std::move(id)) {}

    /**
     * @brief Gets the scenario identifier.
     * @return The string ID of the scenario.
     */
    const std::string& getId() const { return id_; }

    /**
     * @brief Adds a vegetation component to this scenario.
     * @param component The individual vegetation declaration to add.
     */
    void addComponent(const VegetationOriginal& component) {
        components_.push_back(component);
    }

    /**
     * @brief Retrieves all components belonging to this scenario.
     * @return A vector of VegetationOriginal entities.
     */
    const std::vector<VegetationOriginal>& getComponents() const {
        return components_;
    }

    /**
     * @brief Clears all components from this scenario.
     */
    void clear() {
        components_.clear();
    }

private:
    std::string id_;
    std::vector<VegetationOriginal> components_;
};

} // namespace Core::Domain::Vegetation
