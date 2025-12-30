#pragma once

#include <string>

namespace Core::Domain::Vegetation {

/**
 * @brief Value Object representando o Tipo de Vegetação (Campestre ou Florestal).
 * Novos tipos podem ser adicionados, mantendo caráter declarativo.
 */
enum class VegetationCode {
    Campestre,
    FlorestalNatural,
    /**
     * @brief Corpos d'água ou áreas alagadas (Azul).
     */
    Agua
    // Future expansion
};

class VegetationType {
public:
    VegetationType(VegetationCode code) : code_(code) {}

    VegetationCode getCode() const { return code_; }

    std::string toString() const {
        switch (code_) {
            case VegetationCode::Campestre: return "Campestre";
            case VegetationCode::FlorestalNatural: return "FlorestalNatural";
            case VegetationCode::Agua: return "Agua";
            default: return "Unknown";
        }
    }

private:
    VegetationCode code_;
};

} // namespace Core::Domain::Vegetation
