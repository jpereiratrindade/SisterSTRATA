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
    void addHypothesis(const VegetationOriginal& hypothesis) {
        hypotheses_.push_back(hypothesis);
    }

    const std::vector<VegetationOriginal>& getHypotheses() const {
        return hypotheses_;
    }

    void clear() {
        hypotheses_.clear();
    }

private:
    std::vector<VegetationOriginal> hypotheses_;
};

} // namespace Core::Domain::Vegetation
