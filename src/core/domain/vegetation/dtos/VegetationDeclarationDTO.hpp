#pragma once

#include "core/domain/vegetation/VegetationType.hpp"
#include <string>
#include <optional>

namespace Core::Domain::Vegetation::DTOs {

/**
 * @brief Data Transfer Object para encapsular os dados de uma declaração de vegetação.
 * Isola a camada de apresentação da assinatura do serviço de domínio.
 */
struct VegetationDeclarationDTO {
    std::string id;
    VegetationCode typeCode;
    
    // Relief Criteria
    float minSlope = 0.0f;
    float maxSlope = 90.0f;
    float maxDistDrainage = 0.0f; // 0 = ignored

    // Future: Add validation logic or simple data vector accessors if needed
};

} // namespace Core::Domain::Vegetation::DTOs
