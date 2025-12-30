#pragma once

#include "core/domain/vegetation/VegetationSystemOriginal.hpp"
#include "core/domain/vegetation/VegetationOriginal.hpp"
#include "core/domain/vegetation/VegetationType.hpp"
#include "core/domain/vegetation/ReliefCondition.hpp"
#include "core/domain/vegetation/HypothesisID.hpp"

#include "core/domain/vegetation/dtos/VegetationDeclarationDTO.hpp"

namespace Core::Domain::Vegetation {

/**
 * @brief Domain Service.
 * Responsável por criar e validar hipóteses de vegetação.
 */
class VegetationDeclarationService {
public:
    VegetationOriginal createHypothesis(const DTOs::VegetationDeclarationDTO& dto) {
        
        ReliefCondition cond;
        cond.minSlope = dto.minSlope;
        cond.maxSlope = dto.maxSlope;
        if (dto.maxDistDrainage > 0) cond.maxDistanceToDrainage = dto.maxDistDrainage;

        return VegetationOriginal(
            HypothesisID(dto.id),
            VegetationType(dto.typeCode),
            cond
        );
    }
};

} // namespace Core::Domain::Vegetation
