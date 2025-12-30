#pragma once

#include "core/domain/territory/Territory.hpp"
#include "core/domain/land_use/TerritorialHypothesis.hpp"
#include "core/domain/land_use/LandUseCompatibilityService.hpp"
#include "core/domain/spatial_pattern/PatchAnalysis.hpp"
#include <vector>
#include <map>
#include <cmath>
#include <glm/glm.hpp>

namespace Core::Domain::Territory {

/**
 * @brief Domain Service responsible for evaluating the Coherence of a Territorial Hypothesis.
 * 
 * This service acts as the "Judge". It takes a proposed hypothesis and the current territory state 
 * (Bio-Physical reality) and calculates:
 * 1. Bio-Physical Compatibility (Soil/Slope vs Land Use)
 * 2. Spatial Coherence (Fragmentation, Connectivity - Future)
 * 
 * It produces a unified CoherenceScore.
 */
class TerritorialCoherenceService {
public:
    /**
     * @brief Evaluates a hypothesis against a territory.
     * 
     * @param territory The bio-physical reality (Soils, Hydro, Slope).
     * @param hypothesis The user's proposed configuration.
     * @return Shared::ValueObjects::CoherenceScore The resulting score.
     */
    static Shared::ValueObjects::CoherenceScore evaluate(
        const Territory& territory,
        const LandUse::TerritorialHypothesis& hypothesis) 
    {
        // 1. Validation
        const auto& slopeLayer = territory.getSlopeLayer();
        const auto& soilLayer = territory.getSoilLayer();
        
        if (slopeLayer.empty() || soilLayer.empty()) {
            return Shared::ValueObjects::CoherenceScore(0.0f, "Territory data missing (Soil or Slope empty).");
        }
        
        size_t totalCells = slopeLayer.size();
        if (soilLayer.size() != totalCells) {
             return Shared::ValueObjects::CoherenceScore(0.0f, "Data mismatch: Soil and Slope layer sizes differ.");
        }

        // 2. Evaluate Allocation Competition (Priority-Based)
        std::vector<std::string> preliminaryAllocation(totalCells, "");
        const auto& rules = hypothesis.getAllocationRules();
        
        for (size_t i = 0; i < totalCells; ++i) {
            float s = slopeLayer[i];
            int bestPriority = 999;
            std::string winnerId;
            
            for (const auto& rule : rules) {
                if (bestPriority != 999 && rule.priority >= bestPriority) continue;
                
                bool fits = true;
                if (rule.parameters.count("slope_min")) {
                   float minS = std::stof(rule.parameters.at("slope_min"));
                   if (s < minS) fits = false;
                }
                if (fits && rule.parameters.count("slope_max")) {
                   float maxS = std::stof(rule.parameters.at("slope_max"));
                   if (s > maxS) fits = false;
                }
                
                if (fits && rule.parameters.count("soil_order")) {
                    std::string requestedOrder = rule.parameters.at("soil_order");
                    if (requestedOrder != "Qualquer") {
                        std::string currentOrder = Soils::SiBCSHelper::getBaseName(soilLayer[i].order);
                        if (currentOrder != requestedOrder) fits = false;
                    }
                }
                
                if (fits) {
                    bestPriority = rule.priority;
                    winnerId = rule.landUseId;
                }
            }
            preliminaryAllocation[i] = winnerId;
        }

        // 3. NEW: Ecological Refinement (Spatial Context)
        // Group rules by landUseId to check for spatial constraints
        std::map<std::string, double> minPatchSizes;
        for (const auto& rule : rules) {
            if (rule.parameters.count("min_patch_size")) {
                double val = std::stod(rule.parameters.at("min_patch_size"));
                // If multiple rules for same use, take the stricter (higher) min size? Or maybe per rule?
                // Per architecture guidelines, we treat the use as a whole in the meso scale.
                minPatchSizes[rule.landUseId] = std::max(minPatchSizes[rule.landUseId], val);
            }
        }

        std::vector<std::string> finalAllocation = preliminaryAllocation;
        size_t ecologicalResolutionFiltered = 0;

        for (auto const& [landUseId, minSize] : minPatchSizes) {
            if (minSize <= 1.0) continue; // 1 cell is default

            SpatialPattern::GridData grid;
            grid.width = territory.getWidth();
            grid.height = territory.getHeight();
            grid.values.resize(totalCells, 0.0);
            for(size_t i=0; i<totalCells; ++i) {
                if(preliminaryAllocation[i] == landUseId) grid.values[i] = 1.0;
            }

            SpatialPattern::AnalysisConfig cfg;
            cfg.threshold = 0.5;
            cfg.keepLabels = true;
            auto result = SpatialPattern::AnalyzeGrid(grid, cfg);

            // Map cell to patch metrics
            for(size_t i=0; i<totalCells; ++i) {
                uint32_t label = result.labelImage.labels[i];
                if (label > 0) {
                    const auto& patch = result.patches[label - 1];
                    // patch.area is in cells * cellWidth * cellHeight. 
                    // Assume area is comparable to minSize (usually in cells for now unless specified)
                    if (patch.area < minSize) {
                        finalAllocation[i] = ""; // De-allocate
                        ecologicalResolutionFiltered++;
                    }
                }
            }
        }

        // 4. Generate Report
        std::map<std::string, size_t> allocationCounts;
        size_t allocatedTotal = 0;
        for(const auto& id : finalAllocation) {
            if (!id.empty()) {
                allocationCounts[id]++;
                allocatedTotal++;
            }
        }

        std::string report;
        int incoherentUses = 0;
        
        for (const auto& potential : hypothesis.getLandUseTypes()) {
            size_t count = allocationCounts[potential.getId()];
            float pct = (float)count / (float)totalCells;
            int pctInt = (int)(pct * 100.0f);
            
            if (count == 0) {
                bool hasRules = false;
                for(const auto& r : rules) { if(r.landUseId == potential.getId()) hasRules=true; }
                
                if (hasRules) {
                    incoherentUses++;
                    report += "[Fail] " + potential.getName() + ": 0% allocated (Overwritten or Constraints too tight). ";
                } else {
                    report += "[Info] " + potential.getName() + ": No rules defined. ";
                }
            } else {
                report += "[OK] " + potential.getName() + " (" + std::to_string(pctInt) + "% allocated). ";
            }
        }

        if (ecologicalResolutionFiltered > 0) {
            report += "[Ecological] " + std::to_string(ecologicalResolutionFiltered) + " cells below Ecological Resolution. ";
        }

        size_t unallocatedCount = totalCells - allocatedTotal;
        if (unallocatedCount > 0) {
            float unallocatedPct = (float)unallocatedCount / (float)totalCells;
            int unallocatedInt = (int)(unallocatedPct * 100.0f);
            report += "[Info] Unallocated: " + std::to_string(unallocatedInt) + "%. ";
        }

        if (incoherentUses > 0) {
            return Shared::ValueObjects::CoherenceScore(0.4f, "Incoherent: " + report);
        }
        
        return Shared::ValueObjects::CoherenceScore(1.0f, "Coherent: " + report);
    }

    /**
     * @brief Generates a visualization color buffer based on the hypothesis allocation.
     * 
     * @param territory The physical context.
     * @param hypothesis The configuration rules.
     * @return std::vector<glm::vec3> Color buffer (RGB) for each cell/vertex.
     */
    static std::vector<glm::vec3> generateVisualizationColors(
        const Territory& territory,
        const LandUse::TerritorialHypothesis& hypothesis)
    {
        const auto& slopeLayer = territory.getSlopeLayer();
        const auto& soilLayer = territory.getSoilLayer();
        size_t totalCells = slopeLayer.size();
        
        if (slopeLayer.empty() || soilLayer.empty() || slopeLayer.size() != soilLayer.size()) {
            return std::vector<glm::vec3>(totalCells, glm::vec3(1.0f, 0.0f, 1.0f)); // Magenta error
        }

        auto finalAllocation = generateLandUseVector(territory, hypothesis);
        std::vector<glm::vec3> colors;
        colors.reserve(totalCells);

        const auto& potentials = hypothesis.getLandUseTypes();
        std::map<std::string, glm::vec3> colorMap;
        for(const auto& p : potentials) colorMap[p.getId()] = p.getColor();

        for (const auto& id : finalAllocation) {
            if (id.empty()) {
                colors.push_back(glm::vec3(0.1f, 0.1f, 0.1f));
            } else {
                colors.push_back(colorMap[id]);
            }
        }
        
        return colors;
    }

    /**
     * @brief Generates the Land Use ID vector for resilience analysis.
     */
    static std::vector<std::string> generateLandUseVector(
        const Territory& territory,
        const LandUse::TerritorialHypothesis& hypothesis)
    {
        const auto& slopeLayer = territory.getSlopeLayer();
        const auto& soilLayer = territory.getSoilLayer();
        size_t totalCells = slopeLayer.size();
        
        if (slopeLayer.empty() || soilLayer.empty() || slopeLayer.size() != soilLayer.size()) {
            return std::vector<std::string>(totalCells, "");
        }

        std::vector<std::string> allocation(totalCells, "");
        const auto& rules = hypothesis.getAllocationRules();

        for (size_t i = 0; i < totalCells; ++i) {
            float s = slopeLayer[i];
            int bestPriority = 999;
            std::string winnerId;
            
            for (const auto& rule : rules) {
                if (rule.priority >= bestPriority && bestPriority != 999) continue;

                bool fits = true;
                if (rule.parameters.count("slope_min")) {
                    if (s < std::stof(rule.parameters.at("slope_min"))) fits = false;
                }
                if (rule.parameters.count("slope_max")) {
                    if (s > std::stof(rule.parameters.at("slope_max"))) fits = false;
                }
                
                if (fits && rule.parameters.count("soil_order")) {
                    std::string requestedOrder = rule.parameters.at("soil_order");
                    if (requestedOrder != "Qualquer") {
                        std::string currentOrder = Soils::SiBCSHelper::getBaseName(soilLayer[i].order);
                        if (currentOrder != requestedOrder) fits = false;
                    }
                }
                
                if (fits) {
                    winnerId = rule.landUseId;
                    bestPriority = rule.priority;
                }
            }
            allocation[i] = winnerId;
        }

        // Ecological Filter (Consistent with evaluate)
        std::map<std::string, double> minPatchSizes;
        for (const auto& rule : rules) {
            if (rule.parameters.count("min_patch_size")) {
                double val = std::stod(rule.parameters.at("min_patch_size"));
                minPatchSizes[rule.landUseId] = std::max(minPatchSizes[rule.landUseId], val);
            }
        }

        for (auto const& [landUseId, minSize] : minPatchSizes) {
            if (minSize <= 1.0) continue;

            SpatialPattern::GridData grid;
            grid.width = territory.getWidth();
            grid.height = territory.getHeight();
            grid.values.resize(totalCells, 0.0);
            for(size_t i=0; i<totalCells; ++i) {
                if(allocation[i] == landUseId) grid.values[i] = 1.0;
            }

            auto result = SpatialPattern::AnalyzeGrid(grid, {0.5, false, true});

            for(size_t i=0; i<totalCells; ++i) {
                uint32_t label = result.labelImage.labels[i];
                if (label > 0) {
                    if (result.patches[label - 1].area < minSize) {
                        allocation[i] = ""; 
                    }
                }
            }
        }
        
        return allocation;
    }

};

} // namespace Core::Domain::Territory
