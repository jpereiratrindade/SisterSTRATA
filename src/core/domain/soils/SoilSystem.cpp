#include "SoilSystem.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>

namespace Core::Domain::Soils {

void SoilSystem::process(std::vector<World3D::Rendering::Vertex>& vertices, const ScorpanParams& params, int visualizationLevel) {
    if (vertices.empty()) return;

    std::cout << "[SoilSystem] Running SCORPAN Soil Prediction (Level " << visualizationLevel << ")..." << std::endl;

    // 1. Pre-calc stats
    float minZ = 1e9f;
    float maxZ = -1e9f;
    for (const auto& v : vertices) {
        minZ = std::min(minZ, v.pos.z);
        maxZ = std::max(maxZ, v.pos.z);
    }
    if (maxZ == minZ) maxZ = minZ + 1.0f;

    // 2. Iterate and Predict
    for (auto& v : vertices) {
        float dot = std::clamp(v.normal.z, -1.0f, 1.0f);
        float slopeDeg = glm::degrees(std::acos(dot));
        float relElev = (v.pos.z - minZ) / (maxZ - minZ);

        auto classification = predict(params, slopeDeg, v.pos.z, relElev);
        
        // Visualize
        v.color = SiBCSHelper::getColor(classification, visualizationLevel);
    }

    std::cout << "[SoilSystem] Soil Map Generated." << std::endl;
}

SiBCSClassification SoilSystem::predict(const ScorpanParams& global, float slopeDeg, float elevation, float relElevation) {
    SiBCSClassification result;

    // --- LEVEL 1: ORDER ---
    // (Logic simplified for demo)
    if (slopeDeg > 35.0f) {
        result.order = SiBCSOrder::Neossolo;
    } else if (relElevation < 0.05f && global.rainfall > 1200.0f) {
        result.order = SiBCSOrder::Gleissolo; 
    } else if (global.ageFactor > 0.7f && slopeDeg < 8.0f) {
        result.order = SiBCSOrder::Latossolo;
    } else if (slopeDeg > 15.0f && slopeDeg <= 35.0f) {
        result.order = SiBCSOrder::Cambissolo;
    } else if (global.ageFactor > 0.3f && slopeDeg < 20.0f) {
        result.order = SiBCSOrder::Argissolo;
    } else {
        result.order = SiBCSOrder::Cambissolo;
    }

    // --- LEVEL 2: SUBORDER ---
    // Derived from Parent Material and Color/Process
    switch (result.order) {
        case SiBCSOrder::Latossolo:
        case SiBCSOrder::Argissolo:
            if (global.parentMaterial == ParentMaterialType::Igneous) {
                 result.suborder = SiBCSSuborder::Vermelho; // Iron rich
            } else if (global.parentMaterial == ParentMaterialType::Sedimentary) {
                 result.suborder = SiBCSSuborder::Amarelo; // Goethite
            } else {
                 result.suborder = SiBCSSuborder::VermelhoAmarelo; 
            }
            break;
        case SiBCSOrder::Neossolo:
             result.suborder = SiBCSSuborder::Litoc; // Litólico (Stony)
             break;
        case SiBCSOrder::Gleissolo:
             result.suborder = SiBCSSuborder::Gleico; // Redundant but correct context for suborder name
             // Actually Gleissolo suborders are Tiomorfico, Salico, Melânico, Haplico...
             result.suborder = SiBCSSuborder::Haplic;
             break;
        case SiBCSOrder::Cambissolo:
             result.suborder = SiBCSSuborder::Haplic;
             break;
        default:
             result.suborder = SiBCSSuborder::Haplic;
    }

    // --- LEVEL 3: GREAT GROUP ---
    // Derived from Rainfall/Fertility (Saturation)
    // High Rain -> Leaching -> Dystrophic/Alic
    if (global.rainfall > 2000.0f) {
        result.greatGroup = SiBCSGreatGroup::Alico; // Very leached
    } else if (global.rainfall > 1200.0f) {
        result.greatGroup = SiBCSGreatGroup::Distrofico; // Leached
    } else {
        result.greatGroup = SiBCSGreatGroup::Eutrofico; // Dry/Fertile
    }

    // --- LEVEL 4, 5, 6 ---
    result.subgroup = SiBCSSubgroup::Tipico;
    result.family = SiBCSFamily::TexturaMedia; // Placeholder
    result.series = SiBCSSeries::Generica;

    return result;
}

} // namespace Core::Domain::Soils
