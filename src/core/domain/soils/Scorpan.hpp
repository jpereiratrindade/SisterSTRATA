#pragma once

namespace Core::Domain::Soils {

enum class ParentMaterialType {
    Igneous,
    Sedimentary,
    Metamorphic
};

struct ScorpanParams {
    // S = f(s, c, o, r, p, a, n)

    // Global factors (User inputs)
    float rainfall = 1500.0f;       // mm/year (Climate)
    float temperature = 25.0f;      // Celsius (Climate)
    float vegetationDensity = 0.5f; // 0..1 (Organisms)
    float ageFactor = 0.5f;         // 0..1 (0=Young/Recent, 1=Old/Mature) (Age)
    ParentMaterialType parentMaterial = ParentMaterialType::Sedimentary; // (Parent Material)

    // Local factors (Derived from Terrain per vertex)
    // float slope;      // (Relief)
    // float elevation;  // (Relief)
    // These are passed individually during prediction
};

} // namespace Core::Domain::Soils
