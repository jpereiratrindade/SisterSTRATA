#pragma once

#include "application/dtos/SoilDTOs.hpp"
#include "application/services/World3DService.hpp"
#include "core/domain/soils/Scorpan.hpp"
#include "core/domain/soils/SiBCS.hpp"
#include "core/domain/soils/SoilSystem.hpp"
#include "core/domain/spatial_pattern/SoilRasterizer.hpp"
#include "core/value_objects/TerrainVertex.hpp"
#include <algorithm>
#include <cmath>
#include <glm/glm.hpp>

namespace Application::Services {

class SoilAnalysisService {
public:
    static Core::Domain::Soils::ScorpanParams toCore(const Application::DTO::Soils::ScorpanParamsDTO& dto) {
        Core::Domain::Soils::ScorpanParams params;
        params.rainfall = dto.rainfall;
        params.temperature = dto.temperature;
        params.vegetationDensity = dto.vegetationDensity;
        params.ageFactor = dto.ageFactor;
        params.parentMaterial = static_cast<Core::Domain::Soils::ParentMaterialType>(dto.parentMaterial);
        return params;
    }

    static Core::Domain::Soils::SiBCSFilter toCore(const Application::DTO::Soils::SiBCSFilterDTO& dto) {
        Core::Domain::Soils::SiBCSFilter filter;
        for (int v : dto.allowedOrders) filter.allowedOrders.push_back(static_cast<Core::Domain::Soils::SiBCSOrder>(v));
        for (int v : dto.allowedSuborders) filter.allowedSuborders.push_back(static_cast<Core::Domain::Soils::SiBCSSuborder>(v));
        for (int v : dto.allowedGreatGroups) filter.allowedGreatGroups.push_back(static_cast<Core::Domain::Soils::SiBCSGreatGroup>(v));
        for (int v : dto.allowedSubgroups) filter.allowedSubgroups.push_back(static_cast<Core::Domain::Soils::SiBCSSubgroup>(v));
        for (int v : dto.allowedFamilies) filter.allowedFamilies.push_back(static_cast<Core::Domain::Soils::SiBCSFamily>(v));
        for (int v : dto.allowedSeries) filter.allowedSeries.push_back(static_cast<Core::Domain::Soils::SiBCSSeries>(v));
        return filter;
    }

    static void applySoilSimulation(const Application::DTO::Soils::ScorpanParamsDTO& params,
                                    int visualizationLevel,
                                    const Application::DTO::Soils::SiBCSFilterDTO& filter) {
        Application::Services::World3DService::applySoilSimulation(
            toCore(params), visualizationLevel, toCore(filter)
        );
    }

    struct SoilClassResult {
        std::vector<Core::Domain::Soils::SiBCSClassification> classes;
        std::vector<Core::Domain::Soils::SiBCSClassification> uniqueClasses;
    };

    static SoilClassResult classifyTerrain(
        const std::vector<Core::ValueObjects::TerrainVertex>& vertices,
        const Application::DTO::Soils::ScorpanParamsDTO& params
    ) {
        SoilClassResult result;
        if (vertices.empty()) return result;

        const auto coreParams = toCore(params);
        float minZ = 1e9f;
        float maxZ = -1e9f;
        for (const auto& v : vertices) {
            minZ = std::min(minZ, v.pos.z);
            maxZ = std::max(maxZ, v.pos.z);
        }
        if (maxZ == minZ) maxZ = minZ + 1.0f;

        result.classes.reserve(vertices.size());

        for (const auto& v : vertices) {
            float dot = std::clamp(v.normal.z, -1.0f, 1.0f);
            float slopeDeg = glm::degrees(std::acos(dot));
            float relElev = (v.pos.z - minZ) / (maxZ - minZ);

            auto c = Core::Domain::Soils::SoilSystem::predict(coreParams, slopeDeg, v.pos.z, relElev);
            result.classes.push_back(c);

            bool exists = false;
            for (const auto& u : result.uniqueClasses) {
                if (u == c) { exists = true; break; }
            }
            if (!exists) result.uniqueClasses.push_back(c);
        }

        return result;
    }

    static Core::Domain::SpatialPattern::GridData rasterize(
        const std::vector<Core::ValueObjects::TerrainVertex>& vertices,
        const std::vector<Core::Domain::Soils::SiBCSClassification>& classes,
        double cellSize
    ) {
        return Core::Domain::SpatialPattern::SoilRasterizer::Rasterize(vertices, classes, cellSize);
    }

    static void saveLegendCsv(
        const std::string& path,
        const std::vector<Core::Domain::Soils::SiBCSClassification>& uniqueClasses
    ) {
        Core::Domain::SpatialPattern::SoilRasterizer::SaveLegendCsv(path, uniqueClasses);
    }

    static void clearLastDetectedClasses() {
        Core::Domain::Soils::SoilSystem::clearLastDetectedClasses();
    }

    static const std::vector<Core::Domain::Soils::SiBCSClassification>& getLastDetectedClasses() {
        return Core::Domain::Soils::SoilSystem::getLastDetectedClasses();
    }

    static std::vector<Application::DTO::Soils::SoilLegendItemDTO> getDetectedLegendItems(int visualizationLevel) {
        std::vector<Application::DTO::Soils::SoilLegendItemDTO> items;
        const auto& classes = Core::Domain::Soils::SoilSystem::getLastDetectedClasses();
        items.reserve(classes.size());
        for (const auto& soil : classes) {
            glm::vec3 color = Core::Domain::Soils::SiBCSHelper::getColor(soil, visualizationLevel);
            items.push_back({Core::Domain::Soils::SiBCSHelper::getName(soil, visualizationLevel), color.r, color.g, color.b});
        }
        return items;
    }

    static std::vector<Application::DTO::Soils::SoilOptionDTO> getParentMaterials() {
        return {
            {static_cast<int>(Core::Domain::Soils::ParentMaterialType::Igneous), "Igneous"},
            {static_cast<int>(Core::Domain::Soils::ParentMaterialType::Sedimentary), "Sedimentary"},
            {static_cast<int>(Core::Domain::Soils::ParentMaterialType::Metamorphic), "Metamorphic"}
        };
    }

    static std::vector<Application::DTO::Soils::SoilOptionDTO> getOrders() {
        std::vector<Application::DTO::Soils::SoilOptionDTO> items;
        for (auto order : Core::Domain::Soils::SiBCSHelper::getAllOrders()) {
            items.push_back({static_cast<int>(order), Core::Domain::Soils::SiBCSHelper::getBaseName(order)});
        }
        return items;
    }

    static std::vector<Application::DTO::Soils::SoilOptionDTO> getSuborders() {
        std::vector<Application::DTO::Soils::SoilOptionDTO> items;
        for (auto sub : Core::Domain::Soils::SiBCSHelper::getAllSuborders()) {
            items.push_back({static_cast<int>(sub), Core::Domain::Soils::SiBCSHelper::getBaseName(sub)});
        }
        return items;
    }

    static std::vector<Application::DTO::Soils::SoilOptionDTO> getGreatGroups() {
        std::vector<Application::DTO::Soils::SoilOptionDTO> items;
        for (auto group : Core::Domain::Soils::SiBCSHelper::getAllGreatGroups()) {
            items.push_back({static_cast<int>(group), Core::Domain::Soils::SiBCSHelper::getBaseName(group)});
        }
        return items;
    }

    static std::vector<Application::DTO::Soils::SoilOptionDTO> getSubgroups() {
        std::vector<Application::DTO::Soils::SoilOptionDTO> items;
        for (auto sub : Core::Domain::Soils::SiBCSHelper::getAllSubgroups()) {
            items.push_back({static_cast<int>(sub), Core::Domain::Soils::SiBCSHelper::getBaseName(sub)});
        }
        return items;
    }

    static std::vector<Application::DTO::Soils::SoilOptionDTO> getFamilies() {
        std::vector<Application::DTO::Soils::SoilOptionDTO> items;
        for (auto fam : Core::Domain::Soils::SiBCSHelper::getAllFamilies()) {
            items.push_back({static_cast<int>(fam), Core::Domain::Soils::SiBCSHelper::getBaseName(fam)});
        }
        return items;
    }

    static std::vector<Application::DTO::Soils::SoilOptionDTO> getSeries() {
        std::vector<Application::DTO::Soils::SoilOptionDTO> items;
        for (auto series : Core::Domain::Soils::SiBCSHelper::getAllSeries()) {
            items.push_back({static_cast<int>(series), Core::Domain::Soils::SiBCSHelper::getBaseName(series)});
        }
        return items;
    }
};

} // namespace Application::Services
