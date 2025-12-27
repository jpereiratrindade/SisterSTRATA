#include "SoilSystem.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>

namespace Core::Domain::Soils {

void SoilSystem::process(std::vector<World3D::Rendering::Vertex>& vertices, const ScorpanParams& params) {
    if (vertices.empty()) return;

    std::cout << "[SoilSystem] Running SCORPAN Soil Prediction..." << std::endl;

    // 1. Pre-calc stats for Relative Elevation (Normalization)
    float minZ = 1e9f;
    float maxZ = -1e9f;
    for (const auto& v : vertices) {
        minZ = std::min(minZ, v.pos.z);
        maxZ = std::max(maxZ, v.pos.z);
    }
    if (maxZ == minZ) maxZ = minZ + 1.0f;

    // 2. Iterate and Predict
    for (auto& v : vertices) {
        // Relief Factors
        // Up vector is (0,0,1). Dot product with normal = n.z
        float dot = std::clamp(v.normal.z, -1.0f, 1.0f);
        float slopeDeg = glm::degrees(std::acos(dot));
        float relElev = (v.pos.z - minZ) / (maxZ - minZ);

        // Predict
        auto type = predict(params, slopeDeg, v.pos.z, relElev);
        
        // Visualize (Apply Color)
        v.color = SiBCSHelper::getColor(type);
    }

    std::cout << "[SoilSystem] Soil Map Generated." << std::endl;
}

SiBCSClass SoilSystem::predict(const ScorpanParams& global, float slopeDeg, float elevation, float relElevation) {
    // SCORPAN Heuristics logic migrated from SoilPredictor
    
    // 1. Check Relief/Slope (Strongest factor for Neossolo)
    // Steep slope (> 35 deg) usually prevents deep soil formation -> Neossolo Litólico
    if (slopeDeg > 35.0f) {
        return SiBCSClass::Neossolo;
    }

    // 2. Hydromorphic check (Low relative elevation + High Rainfall = Gleissolo)
    // relElevation < 0.05 (Depression/Valley bottom)
    bool potentiallyHydromorphic = (relElevation < 0.05f); 
    if (potentiallyHydromorphic && global.rainfall > 1200.0f) {
        return SiBCSClass::Gleissolo; 
    }

    // 3. Age & Weathering check
    // Old soils (Latossolo) require flatness and time
    if (global.ageFactor > 0.7f && slopeDeg < 8.0f) {
        return SiBCSClass::Latossolo;
    }

    // 4. Transitional
    if (slopeDeg > 15.0f && slopeDeg <= 35.0f) {
        return SiBCSClass::Cambissolo; // Shallow but developing
    }
    
    // 5. Argissolo (Bt horizon)
    // Needs some seasonality and age, but not as weathered as Latossolo
    if (global.ageFactor > 0.3f && slopeDeg < 20.0f) {
        return SiBCSClass::Argissolo;
    }

    // Default to Cambissolo if nothing else fits well
    return SiBCSClass::Cambissolo;
}

} // namespace Core::Domain::Soils
