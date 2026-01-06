#pragma once

#include "core/domain/vegetation/EcologicalScenario.hpp"
#include <vector>
#include <memory>
#include <algorithm>
#include <map>

namespace Core::Domain::Vegetation {

/**
 * @brief Aggregate Root.
 * Mantém a coleção de cenários ecológicos (conjuntos de hipóteses agrupados por ID).
 * NÃO executa simulação. NÃO evolui no tempo.
 */
class VegetationSystemOriginal {
public:
    /**
     * @brief Registers a new vegetation hypothesis component.
     * Automatically groups it into the corresponding EcologicalScenario by ID.
     * @param component The vegetation component to add.
     */
    void addHypothesis(const VegetationOriginal& component) {
        std::string id = component.getId().getValue();
        auto it = std::find_if(scenarios_.begin(), scenarios_.end(),
            [&id](const EcologicalScenario& s) { return s.getId() == id; });

        if (it == scenarios_.end()) {
            scenarios_.emplace_back(id);
            scenarios_.back().addComponent(component);
        } else {
            it->addComponent(component);
        }
    }

    /**
     * @brief Retrieves all declared scenarios.
     * @return A vector of EcologicalScenario objects.
     */
    const std::vector<EcologicalScenario>& getScenarios() const {
        return scenarios_;
    }

    /**
     * @brief Backward compatibility for a flat list of components.
     * Iterates through all scenarios and collects all components.
     * @return A flat vector of all VegetationOriginal entities.
     */
    const std::vector<VegetationOriginal> getFlattenedHypotheses() const {
        std::vector<VegetationOriginal> flat;
        for (const auto& s : scenarios_) {
            for (const auto& comp : s.getComponents()) {
                flat.push_back(comp);
            }
        }
        return flat;
    }

    /**
     * @brief Clears all scenarios and hypotheses.
     */
    void clear() {
        scenarios_.clear();
    }

    /**
     * @brief Removes a scenario by its ID.
     * @param id The scenario ID to remove.
     * @return true if found and removed, false otherwise.
     */
    bool removeScenario(const std::string& id) {
        auto it = std::remove_if(scenarios_.begin(), scenarios_.end(),
            [&id](const EcologicalScenario& h) {
                return h.getId() == id;
            });
        
        bool found = (it != scenarios_.end());
        scenarios_.erase(it, scenarios_.end());
        return found;
    }

    /**
     * @brief Removes a specific scenario by its position index.
     * @param index The 0-based index of the scenario.
     * @return true if success, false if index is out of bounds.
     */
    bool removeScenarioByIndex(size_t index) {
        if (index >= scenarios_.size()) return false;
        scenarios_.erase(scenarios_.begin() + index);
        return true;
    }

    /**
     * @brief Swaps the position of two scenarios for priority reordering.
     * @param indexA First scenario index.
     * @param indexB Second scenario index.
     * @return true if success, false if indices are invalid.
     */
    bool swapScenarios(size_t indexA, size_t indexB) {
        if (indexA >= scenarios_.size() || indexB >= scenarios_.size()) return false;
        std::swap(scenarios_[indexA], scenarios_[indexB]);
        return true;
    }

private:
    std::vector<EcologicalScenario> scenarios_;
};

} // namespace Core::Domain::Vegetation
