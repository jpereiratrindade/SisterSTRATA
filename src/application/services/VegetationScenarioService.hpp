#pragma once

#include "application/dtos/VegetationDTOs.hpp"
#include "application/services/VegetationAnalysisService.hpp"
#include "application/services/World3DService.hpp"
#include "core/domain/vegetation/VegetationDeclarationService.hpp"
#include "core/domain/vegetation/VegetationPersistenceService.hpp"
#include "core/domain/vegetation/VegetationSystemOriginal.hpp"
#include "core/domain/fourth_dimension/TrajectoryService.hpp"
#include "core/domain/fourth_dimension/Trajectory.hpp"

namespace Application::Services {

class VegetationScenarioService {
public:
    VegetationScenarioService() {
        refreshScenarioCache();
    }

    void declareHypothesis(const Application::DTO::Vegetation::DeclarationDTO& dto) {
        Core::Domain::Vegetation::DTOs::VegetationDeclarationDTO core;
        core.id = dto.id;
        core.typeCode = static_cast<Core::Domain::Vegetation::VegetationCode>(dto.typeCode);
        core.minSlope = dto.minSlope.value_or(0.0f);
        core.maxSlope = dto.maxSlope.value_or(90.0f);
        core.maxDistDrainage = dto.maxDistDrainage.value_or(0.0f);

        auto hypo = declarationService_.createHypothesis(core);
        system_.addHypothesis(hypo);
        scenarioOutdated_ = true;
        refreshScenarioCache();
    }

    void swapScenarios(size_t a, size_t b) {
        system_.swapScenarios(a, b);
        scenarioOutdated_ = true;
        refreshScenarioCache();
    }

    void removeScenarioByIndex(size_t idx) {
        system_.removeScenarioByIndex(idx);
        scenarioOutdated_ = true;
        refreshScenarioCache();
    }

    const std::vector<Application::DTO::Vegetation::ScenarioDTO>& getScenarioDTOs() const {
        return scenarioCache_;
    }

    Application::DTO::Vegetation::CoverageResultDTO calculatePotentialCoverage(
        size_t scenarioIndex,
        size_t componentIndex,
        const std::vector<Core::ValueObjects::TerrainVertex>& vertices,
        const Core::Domain::Hydro::HydroGrid& hydro,
        float gridSpacing
    ) {
        Application::DTO::Vegetation::CoverageResultDTO result;
        if (scenarioIndex >= system_.getScenarios().size()) return result;
        const auto& scenario = system_.getScenarios()[scenarioIndex];
        const auto& components = scenario.getComponents();
        if (componentIndex >= components.size()) return result;

        auto mapping = Application::Services::VegetationAnalysisService::calculatePotentialCoverage(
            components[componentIndex], vertices, hydro, gridSpacing
        );
        result.matchVertices = mapping.matchVertices;
        result.coveragePercentage = mapping.coveragePercentage;
        result.coverageMask = std::move(mapping.coverageMask);
        return result;
    }

    std::vector<int> resolveScenarioToCodes(
        size_t scenarioIndex,
        const std::vector<Core::ValueObjects::TerrainVertex>& vertices,
        const Core::Domain::Hydro::HydroGrid& hydro,
        float gridSpacing
    ) {
        if (scenarioIndex >= system_.getScenarios().size()) return {};
        const auto& scenario = system_.getScenarios()[scenarioIndex];
        auto codes = Application::Services::VegetationAnalysisService::resolveScenarioToCodes(
            scenario, vertices, hydro, gridSpacing
        );
        lastSemanticClassification_ = codes;
        semanticActive_ = true;
        scenarioOutdated_ = true;
        return codes;
    }

    Application::DTO::Vegetation::ScenarioResultDTO calculateScenario(
        const std::vector<Core::ValueObjects::TerrainVertex>& vertices,
        const Core::Domain::Hydro::HydroGrid& hydro,
        float gridSpacing
    ) {
        Application::DTO::Vegetation::ScenarioResultDTO dto;
        auto result = Application::Services::VegetationAnalysisService::calculateScenario(
            system_.getScenarios(), vertices, hydro, gridSpacing
        );
        lastScenarioResult_ = result;
        scenarioOutdated_ = false;
        semanticActive_ = false;

        dto.classification = result.classification;
        dto.semanticCodes = result.semanticCodes;
        dto.realizedPercentages.reserve(result.stats.size());
        for (const auto& s : result.stats) dto.realizedPercentages.push_back(s.realizedPercentage);
        return dto;
    }

    const Application::DTO::Vegetation::ScenarioResultDTO* getLastScenarioResult() const {
        if (scenarioOutdated_) return nullptr;
        cachedScenarioDTO_ = Application::DTO::Vegetation::ScenarioResultDTO{};
        cachedScenarioDTO_.classification = lastScenarioResult_.classification;
        cachedScenarioDTO_.semanticCodes = lastScenarioResult_.semanticCodes;
        cachedScenarioDTO_.realizedPercentages.reserve(lastScenarioResult_.stats.size());
        for (const auto& s : lastScenarioResult_.stats) cachedScenarioDTO_.realizedPercentages.push_back(s.realizedPercentage);
        return &cachedScenarioDTO_;
    }

    const std::vector<int>& getLastSemanticClassification() const {
        return lastSemanticClassification_;
    }

    bool isSemanticClassificationActive() const {
        return semanticActive_;
    }

    void applyVegetationVisualization(size_t scenarioIndex, size_t componentIndex, const std::vector<bool>& mask) {
        if (scenarioIndex >= system_.getScenarios().size()) return;
        const auto& scenario = system_.getScenarios()[scenarioIndex];
        const auto& components = scenario.getComponents();
        if (componentIndex >= components.size()) return;
        Application::Services::World3DService::applyVegetationVisualization(components[componentIndex], mask);
    }

    void applyClassificationVisualization(const std::vector<int>& semanticMap) {
        Application::Services::World3DService::applyClassificationVisualization(semanticMap);
    }

    void saveScenarios(const std::string& path) {
        Core::Domain::Vegetation::VegetationPersistenceService::saveScenarios(system_, path);
    }

    void loadScenarios(const std::string& path) {
        Core::Domain::Vegetation::VegetationPersistenceService::loadScenarios(system_, path);
        scenarioOutdated_ = true;
        refreshScenarioCache();
    }

    void captureScenarioState(Core::Domain::FourthDimension::Trajectory& trajectory,
                              const std::vector<int>& scenarioIndices,
                              const std::vector<bool>& waterMask,
                              const std::string& metadata) {
        Core::Domain::FourthDimension::TrajectoryService::captureState(
            trajectory, scenarioIndices, system_, waterMask, metadata
        );
    }

private:
    void refreshScenarioCache() {
        scenarioCache_.clear();
        for (const auto& scenario : system_.getScenarios()) {
            Application::DTO::Vegetation::ScenarioDTO dto;
            dto.id = scenario.getId();
            for (const auto& comp : scenario.getComponents()) {
                Application::DTO::Vegetation::ComponentDTO c;
                c.typeLabel = comp.getType().toString();
                c.minSlope = comp.getConditions().minSlope;
                c.maxSlope = comp.getConditions().maxSlope;
                c.maxDistanceToDrainage = comp.getConditions().maxDistanceToDrainage;
                dto.components.push_back(std::move(c));
            }
            scenarioCache_.push_back(std::move(dto));
        }
    }

    Core::Domain::Vegetation::VegetationSystemOriginal system_;
    Core::Domain::Vegetation::VegetationDeclarationService declarationService_;
    Application::Services::VegetationAnalysisService::ScenarioResult lastScenarioResult_;
    std::vector<int> lastSemanticClassification_;
    bool scenarioOutdated_ = true;
    bool semanticActive_ = false;
    std::vector<Application::DTO::Vegetation::ScenarioDTO> scenarioCache_;

    mutable Application::DTO::Vegetation::ScenarioResultDTO cachedScenarioDTO_;
};

} // namespace Application::Services
