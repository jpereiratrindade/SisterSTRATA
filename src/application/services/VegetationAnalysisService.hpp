#pragma once

#include "core/domain/vegetation/VegetationMappingService.hpp"
#include "core/value_objects/TerrainVertex.hpp"

namespace Application::Services {

class VegetationAnalysisService {
public:
    using MappingResult = Core::Domain::Vegetation::VegetationMappingService::MappingResult;
    using ScenarioResult = Core::Domain::Vegetation::VegetationMappingService::ScenarioResult;

    static MappingResult calculatePotentialCoverage(
        const Core::Domain::Vegetation::VegetationOriginal& hypothesis,
        const std::vector<Core::ValueObjects::TerrainVertex>& vertices,
        const Core::Domain::Hydro::HydroGrid& hydro,
        float gridSpacing
    ) {
        return Core::Domain::Vegetation::VegetationMappingService::calculatePotentialCoverage(
            hypothesis, vertices, hydro, gridSpacing
        );
    }

    static ScenarioResult calculateScenario(
        const std::vector<Core::Domain::Vegetation::EcologicalScenario>& scenarios,
        const std::vector<Core::ValueObjects::TerrainVertex>& vertices,
        const Core::Domain::Hydro::HydroGrid& hydro,
        float gridSpacing
    ) {
        return Core::Domain::Vegetation::VegetationMappingService::calculateScenario(
            scenarios, vertices, hydro, gridSpacing
        );
    }

    static std::vector<int> resolveScenarioToCodes(
        const Core::Domain::Vegetation::EcologicalScenario& scenario,
        const std::vector<Core::ValueObjects::TerrainVertex>& vertices,
        const Core::Domain::Hydro::HydroGrid& hydro,
        float gridSpacing
    ) {
        return Core::Domain::Vegetation::VegetationMappingService::resolveScenarioToCodes(
            scenario, vertices, hydro, gridSpacing
        );
    }
};

} // namespace Application::Services
