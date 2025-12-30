#pragma once

#include "core/domain/vegetation/VegetationType.hpp"
#include "core/domain/vegetation/ReliefCondition.hpp"
#include "core/domain/vegetation/HypothesisID.hpp"
#include <vector>

namespace Core::Domain::Vegetation {

/**
 * @brief Entidade Principal. Representa a vegetação potencial declarada.
 * A entidade é IMUTÁVEL após criada.
 */
class VegetationOriginal {
public:
    VegetationOriginal(HypothesisID id, 
                       VegetationType type, 
                       ReliefCondition conditions)
        : id_(std::move(id)), type_(type), conditions_(conditions) {}

    const HypothesisID& getId() const { return id_; }
    const VegetationType& getType() const { return type_; }
    const ReliefCondition& getConditions() const { return conditions_; }

    // Spatial Distribution (Grid representation or similar) could be stored here or computed on demand.
    // For "Declaration", storing the conditions is paramount. The realized distribution is a derivative.
    // However, the DDD says "Attributes: SpatialDistribution".
    // We will placeholder explicit distribution storage for now or keep it implicitly defined by conditions.
    // Let's assume we store "Declarative Criteria" as the primary truth.

private:
    HypothesisID id_;
    VegetationType type_;
    ReliefCondition conditions_;
};

} // namespace Core::Domain::Vegetation
