#pragma once

#include "core/domain/vegetation/VegetationOriginal.hpp"
#include <vector>
#include <memory>

namespace Core::Domain::Vegetation {

/**
 * @brief Aggregate Root.
 * Mantém a coleção de hipóteses de vegetação declaradas.
 * NÃO executa simulação. NÃO evolui no tempo.
 */
class VegetationSystemOriginal {
public:
    /**
     * @brief Registers a new vegetation hypothesis.
     * @param hypothesis The immutable hypothesis entity.
     */
    void addHypothesis(const VegetationOriginal& hypothesis) {
        hypotheses_.push_back(hypothesis);
    }

    /**
     * @brief Retrieves all declared hypotheses.
     * @return Const reference to the list of hypotheses.
     */
    const std::vector<VegetationOriginal>& getHypotheses() const {
        return hypotheses_;
    }

    void clear() {
        hypotheses_.clear();
    }

    /**
     * @brief Removes all hypotheses with the given ID.
     * @param id The ID of the hypothesis to remove.
     * @return true if any hypothesis was removed.
     */
    bool removeHypothesis(const std::string& id) {
        auto it = std::remove_if(hypotheses_.begin(), hypotheses_.end(),
            [&id](const VegetationOriginal& h) {
                return h.getId().getValue() == id;
            });
        
        bool found = (it != hypotheses_.end());
        hypotheses_.erase(it, hypotheses_.end());
        return found;
    }

    /**
     * @brief Removes a specific hypothesis entry by index.
     * @param index The index in the storage vector.
     * @return true if successful.
     */
    bool removeHypothesisByIndex(size_t index) {
        if (index >= hypotheses_.size()) return false;
        hypotheses_.erase(hypotheses_.begin() + index);
        return true;
    }

    /**
     * @brief Swaps two hypotheses by index.
     */
    bool swapHypotheses(size_t indexA, size_t indexB) {
        if (indexA >= hypotheses_.size() || indexB >= hypotheses_.size()) return false;
        std::swap(hypotheses_[indexA], hypotheses_[indexB]);
        return true;
    }

private:
    std::vector<VegetationOriginal> hypotheses_;
};

} // namespace Core::Domain::Vegetation
