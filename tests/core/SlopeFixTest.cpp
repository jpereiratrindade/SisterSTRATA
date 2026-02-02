#include <iostream>
#include <cassert>
#include <vector>
#include <cmath>
#include <glm/glm.hpp>
#include "core/domain/soils/SoilSystem.hpp"
#include "core/domain/shared/SlopeHelper.hpp"
#include "core/value_objects/TerrainVertex.hpp"

using namespace Core::Domain::Soils;
using namespace Core::Domain::Shared;
using namespace Core::ValueObjects;

void test_ramp_slope() {
    // Create a 10x10 ramp
    // Z goes from 0 to 18 along X (spacing = 1.0)
    // dz/dx = 2.0 -> slopeDeg = atan(2) = 63.4 degrees
    std::vector<TerrainVertex> vertices;
    int w = 10, h = 10;
    for (int i = 0; i < w; i++) {
        for (int j = 0; j < h; j++) {
            TerrainVertex v;
            v.pos = glm::vec3((float)i, (float)j, (float)i * 2.0f); 
            v.normal = glm::vec3(0.0f, 0.0f, 1.0f); // UNIFORM UP!
            v.color = glm::vec3(1.0f, 1.0f, 1.0f);
            v.uv = glm::vec2(0.0f, 0.0f);
            vertices.push_back(v);
        }
    }

    // Process with SoilSystem
    ScorpanParams params;
    params.rainfall = 1000.0f;
    params.temperature = 25.0f;
    params.vegetationDensity = 0.5f;
    params.ageFactor = 0.5f;
    params.parentMaterial = ParentMaterialType::Igneous;
    
    SiBCSFilter filter; // All allowed
    
    std::cout << "[Test] Processing ramp with uniform normals (dz/dx=2.0)..." << std::endl;
    SoilSystem::process(vertices, params, 6, filter);
    
    const auto& classes = SoilSystem::getLastDetectedClasses();
    bool foundNeossolo = false;
    for (const auto& c : classes) {
        // Neossolo is predicted for slopeDeg > 35.0f
        if (c.order == SiBCSOrder::Neossolo) {
            foundNeossolo = true;
            break;
        }
    }
    
    if (!foundNeossolo) {
        std::cerr << "[FAIL] Neossolo not detected on a steep ramp with uniform normals." << std::endl;
        assert(foundNeossolo);
    }
    std::cout << "[SUCCESS] Neossolo detected correctly via height-based fallback!" << std::endl;
}

int main() {
    try {
        test_ramp_slope();
        std::cout << "SlopeFixTest: ALL TESTS PASSED." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "SlopeFixTest: EXCEPTION: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
