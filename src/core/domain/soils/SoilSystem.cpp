#include "SoilSystem.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include "core/domain/shared/SlopeHelper.hpp"

namespace Core::Domain::Soils {

// Define static member
std::vector<SiBCSClassification> SoilSystem::lastDetectedClasses_;
std::vector<SiBCSClassification> SoilSystem::lastClassMap_;

void SoilSystem::process(std::vector<Core::ValueObjects::TerrainVertex>& vertices, const ScorpanParams& params, int visualizationLevel, const SiBCSFilter& filter) {
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
    lastDetectedClasses_.clear(); // Clear previous
    lastClassMap_.assign(vertices.size(), SiBCSClassification{});
    std::vector<SiBCSClassification> frameClasses; 

    // 1.5 Detect if we need a height-based fallback (e.g. for Point Clouds/CSVs with uniform normals)
    auto gridInfo = Core::Domain::Shared::SlopeHelper::detectGrid(vertices);
    bool useHeightFallback = gridInfo.isValid && Core::Domain::Shared::SlopeHelper::areNormalsUniformUp(vertices);

    if (useHeightFallback) {
        std::cout << "[SoilSystem] Uniform normals detected with grid structure. Using height-based slope fallback." << std::endl;
    }

    for (size_t idx = 0; idx < vertices.size(); ++idx) {
        auto& v = vertices[idx];
        float slopeDeg = 0.0f;

        if (useHeightFallback) {
            int gx = static_cast<int>(idx / static_cast<size_t>(gridInfo.height));
            int gy = static_cast<int>(idx % static_cast<size_t>(gridInfo.height));
            slopeDeg = Core::Domain::Shared::SlopeHelper::calculateSlopeDeg(vertices, gridInfo, gx, gy);
        } else {
            float dot = std::clamp(v.normal.z, -1.0f, 1.0f);
            slopeDeg = glm::degrees(std::acos(dot));
        }

        float relElev = (v.pos.z - minZ) / (maxZ - minZ);

        auto classification = predict(params, slopeDeg, v.pos.z, relElev);
        lastClassMap_[idx] = classification;
        
        // Filter Check
        if (!SiBCSHelper::matches(classification, filter)) {
            v.color = glm::vec3(0.3f, 0.3f, 0.3f); 
            continue;
        }
        
        // Collect UNIQUE classes for Legend (only if they pass filter!)
        // Simple linear check is fine for typical counts ( < 20 classes)
        bool exists = false;
        for(const auto& c : frameClasses) if(c == classification) { exists = true; break; }
        if (!exists) frameClasses.push_back(classification);

        // Visualize
        v.color = SiBCSHelper::getColor(classification, visualizationLevel);
    }
    
    // Assign to static (sorted makes legend nicer)
    lastDetectedClasses_ = frameClasses;

    std::cout << "[SoilSystem] Soil Map Generated." << std::endl;
    std::cout << "[SoilSystem] === Detected Classes in Simulation ===" << std::endl;
    for (const auto& c : lastDetectedClasses_) {
        std::cout << "  - " << SiBCSHelper::getName(c, 6) << std::endl;
    }
    std::cout << "============================================" << std::endl;
}

const std::vector<SiBCSClassification>& SoilSystem::getLastDetectedClasses() {
    return lastDetectedClasses_;
}

const std::vector<SiBCSClassification>& SoilSystem::getLastClassMap() {
    return lastClassMap_;
}

void SoilSystem::clearLastDetectedClasses() {
    lastDetectedClasses_.clear();
    lastClassMap_.clear();
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
