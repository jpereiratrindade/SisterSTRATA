#pragma once

namespace Core::Domain::Soils {

enum class ParentMaterialType {
    Igneous,
    Sedimentary,
    Metamorphic
};

/**
 * @brief Represents the global soil forming factors (Inputs).
 * Based on the SCORPAN model: Soil = f(s, c, o, r, p, a, n).
 */
struct ScorpanParams {
    // S = f(s, c, o, r, p, a, n)

    // Global factors (User inputs)
    float rainfall = 1500.0f;       ///< Annual rainfall in mm (Climate)
    float temperature = 25.0f;      ///< Average temperature in Celsius (Climate)
    float vegetationDensity = 0.5f; ///< 0.0 to 1.0 (Organisms)
    float ageFactor = 0.5f;         ///< 0.0 (Young) to 1.0 (Old/Mature) (Age)
    ParentMaterialType parentMaterial = ParentMaterialType::Sedimentary; ///< Lithology (Parent Material)

    // Local factors (Derived from Terrain per vertex):
    // Relief (r) is calculated dynamically from slope and elevation.
    // Position (n) is implicit in vertex coordinates.
};

} // namespace Core::Domain::Soils
