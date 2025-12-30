#pragma once

#include <optional>

namespace Core::Domain::Vegetation {

/**
 * @brief Representa critérios derivados do relevo.
 * NÃO é regra causal, apenas critério de plausibilidade espacial.
 */
struct ReliefCondition {
    // Declividade (Slope)
    std::optional<float> minSlope;
    std::optional<float> maxSlope;

    // Altimetria (Elevation) - Future use?
    // Curvatura (Curvature) - Future use?
    
    // Proximidade de Drenagem (Hydro Dist)
    std::optional<float> maxDistanceToDrainage;

    /**
     * @brief Checks if a specific point in the terrain satisfies the relief conditions.
     * @param slope The local slope in degrees.
     * @param distToBrainage distance to nearest drainage line (meters).
     * @return true if consistent with hypothesis.
     */
    bool satisfies(float slope, float distToBrainage) const {
        if (minSlope.has_value() && slope < minSlope.value()) return false;
        if (maxSlope.has_value() && slope > maxSlope.value()) return false;
        if (maxDistanceToDrainage.has_value() && distToBrainage > maxDistanceToDrainage.value()) return false;
        return true;
    }
};

} // namespace Core::Domain::Vegetation
