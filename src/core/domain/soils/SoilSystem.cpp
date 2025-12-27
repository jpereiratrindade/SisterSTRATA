#include "SoilSystem.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>

namespace Core::Domain::Soils {

void SoilSystem::process(std::vector<World3D::Rendering::Vertex>& vertices, const ScorpanParams& params, int visualizationLevel, const SiBCSFilter& filter) {
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
        
        // Filter Check
        if (!SiBCSHelper::matches(classification, filter)) {
            // Mask out: make it neutral grey to indicate "not selected"
            v.color = glm::vec3(0.3f, 0.3f, 0.3f); 
            continue;
        }

        // Visualize
        v.color = SiBCSHelper::getColor(classification, visualizationLevel);
    }

    std::cout << "[SoilSystem] Soil Map Generated." << std::endl;
}

SiBCSClassification SoilSystem::predict(const ScorpanParams& global, float slopeDeg, float elevation, float relElevation) {
    SiBCSClassification result;

    // --- LEVEL 1: ORDER (Base) ---
    // Determined primarily by Relief (Slope) and Age
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

    // --- LEVEL 2: SUBORDER (Cumulative) ---
    // Depends on Order + Parent Material/Color
    switch (result.order) {
        case SiBCSOrder::Latossolo:
        case SiBCSOrder::Argissolo:
            // Color is dominant for these orders
            if (global.parentMaterial == ParentMaterialType::Igneous) {
                 result.suborder = SiBCSSuborder::Vermelho; // Iron rich
            } else if (global.parentMaterial == ParentMaterialType::Sedimentary) {
                 result.suborder = SiBCSSuborder::Amarelo; // Goethite
            } else {
                 result.suborder = SiBCSSuborder::VermelhoAmarelo; 
            }
            break;
            
        case SiBCSOrder::Neossolo:
             // Lithic contact is dominant
             result.suborder = SiBCSSuborder::Litoc; 
             break;
             
        case SiBCSOrder::Gleissolo:
             // Morphological features
             result.suborder = SiBCSSuborder::Haplic;
             break;
             
        default:
             result.suborder = SiBCSSuborder::Haplic;
    }

    // --- LEVEL 3: GREAT GROUP (Cumulative) ---
    // Depends on Suborder + Climate (Rainfall)
    // High Rain -> Leaching -> Dystrophic
    bool isLeached = global.rainfall > 1200.0f;
    bool isVeryLeached = global.rainfall > 2000.0f;

    if (result.order == SiBCSOrder::Neossolo) {
        // Neossolos Litólicos are usually Eutrophic (fresh rock) unless very specifically leached
        result.greatGroup = isLeached ? SiBCSGreatGroup::Distrofico : SiBCSGreatGroup::Eutrofico;
    } else {
        if (isVeryLeached) {
            result.greatGroup = SiBCSGreatGroup::Alico; 
        } else if (isLeached) {
            result.greatGroup = SiBCSGreatGroup::Distrofico;
        } else {
            result.greatGroup = SiBCSGreatGroup::Eutrofico;
        }
    }

    // --- LEVEL 4, 5, 6 ---
    result.subgroup = SiBCSSubgroup::Tipico;
    result.family = SiBCSFamily::TexturaMedia; // Placeholder
    result.series = SiBCSSeries::Generica;

    return result;
}

} // namespace Core::Domain::Soils
