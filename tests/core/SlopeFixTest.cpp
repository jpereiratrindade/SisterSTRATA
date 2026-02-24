#include <vector>

#include <gtest/gtest.h>

#include <glm/glm.hpp>

#include "core/domain/soils/SoilSystem.hpp"
#include "core/domain/shared/SlopeHelper.hpp"
#include "core/value_objects/TerrainVertex.hpp"

using namespace Core::Domain::Soils;
using namespace Core::ValueObjects;

TEST(CoreSlopeFixTest, DetectsNeossoloOnSteepRampWithUniformNormals) {
    std::vector<TerrainVertex> vertices;
    const int w = 10;
    const int h = 10;

    for (int i = 0; i < w; ++i) {
        for (int j = 0; j < h; ++j) {
            TerrainVertex v;
            v.pos = glm::vec3(static_cast<float>(i), static_cast<float>(j), static_cast<float>(i) * 2.0f);
            v.normal = glm::vec3(0.0f, 0.0f, 1.0f);
            v.color = glm::vec3(1.0f, 1.0f, 1.0f);
            v.uv = glm::vec2(0.0f, 0.0f);
            vertices.push_back(v);
        }
    }

    ScorpanParams params;
    params.rainfall = 1000.0f;
    params.temperature = 25.0f;
    params.vegetationDensity = 0.5f;
    params.ageFactor = 0.5f;
    params.parentMaterial = ParentMaterialType::Igneous;

    SiBCSFilter filter;

    SoilSystem::process(vertices, params, 6, filter);

    const auto& classes = SoilSystem::getLastDetectedClasses();
    bool foundNeossolo = false;
    for (const auto& c : classes) {
        if (c.order == SiBCSOrder::Neossolo) {
            foundNeossolo = true;
            break;
        }
    }

    EXPECT_TRUE(foundNeossolo);
}
