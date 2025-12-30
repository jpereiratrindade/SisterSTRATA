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
        std::map<std::string, size_t> allocationCounts;
        size_t allocatedTotal = 0;
        const auto& rules = hypothesis.getAllocationRules();
        
        // Parallelize? No, keep simple for now or use OpenMP
        for (size_t i = 0; i < totalCells; ++i) {
            float s = slopeLayer[i];
            
            int bestPriority = 999;
            std::string winnerId;
            
            for (const auto& rule : rules) {
                // Optimization: Skip if we already have a better priority match
                if (bestPriority != 999 && rule.priority >= bestPriority) continue;
                
                bool fits = true;
                // Check Parameters
                if (rule.parameters.count("slope_min")) {
                   float minS = std::stof(rule.parameters.at("slope_min"));
                   if (s < minS) fits = false;
                }
                if (fits && rule.parameters.count("slope_max")) {
                   float maxS = std::stof(rule.parameters.at("slope_max"));
                   if (s > maxS) fits = false;
                }
                
                if (fits) {
                    bestPriority = rule.priority;
                    winnerId = rule.landUseId;
                }
            }
            
            if (!winnerId.empty()) {
                allocationCounts[winnerId]++;
                allocatedTotal++;
            }
        }

        // 3. Generate Report
        std::string report;
        int incoherentUses = 0;
        
        for (const auto& potential : hypothesis.getLandUseTypes()) {
            size_t count = allocationCounts[potential.getId()];
            float pct = (float)count / (float)totalCells;
            
            // Format percentage
            int pctInt = (int)(pct * 100.0f);
            
            if (count == 0) {
                // Determine if this is a failure. 
                // If a LandUse exists in hypothesis but gets 0 allocation:
                // Check if it even HAS rules.
                bool hasRules = false;
                for(const auto& r : rules) { if(r.landUseId == potential.getId()) hasRules=true; }
                
                if (hasRules) {
                    incoherentUses++;
                    report += "[Fail] " + potential.getName() + ": 0% allocated (Overwritten by higher priority?). ";
                } else {
                    report += "[Info] " + potential.getName() + ": No rules defined. ";
                }
            } else {
                report += "[OK] " + potential.getName() + " (" + std::to_string(pctInt) + "% allocated). ";
            }
        }

        // Report Unallocated/Constrained
        size_t unallocatedCount = totalCells - allocatedTotal;
        if (unallocatedCount > 0) {
            float unallocatedPct = (float)unallocatedCount / (float)totalCells;
            int unallocatedInt = (int)(unallocatedPct * 100.0f);
            report += "[Info] Unallocated/Constrained: " + std::to_string(unallocatedInt) + "%. ";
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
        size_t count = slopeLayer.size();
        
        std::vector<glm::vec3> colors;
        colors.reserve(count);
        
        // Pre-fetch potentials for lookup
        const auto& potentials = hypothesis.getLandUseTypes();
        
        if (slopeLayer.empty() || soilLayer.empty() || slopeLayer.size() != soilLayer.size()) {
            // Return empty or error colors
            return std::vector<glm::vec3>(slopeLayer.size(), glm::vec3(1.0f, 0.0f, 1.0f)); // Magenta error
        }
        
        for (size_t i = 0; i < count; ++i) {
            float s = slopeLayer[i];
            const auto& soil = soilLayer[i];
            
            // Default: No Allocation (Black/Dark Grey)
            glm::vec3 assignedColor(0.1f, 0.1f, 0.1f);
            int bestPriority = 999;
            
            // Find best matching rule
            for (const auto& rule : hypothesis.getAllocationRules()) {
                if (rule.priority > bestPriority) continue; // Optimization: only check better or equal priority (assuming lower # is better?)
                // Actually strictly better: <
                if (rule.priority >= bestPriority && bestPriority != 999) continue;

                // Check constraints
                bool fits = true;
                if (rule.parameters.count("slope_min")) {
                    if (s < std::stof(rule.parameters.at("slope_min"))) fits = false;
                }
                if (rule.parameters.count("slope_max")) {
                    if (s > std::stof(rule.parameters.at("slope_max"))) fits = false;
                }
                
                if (fits) {
                    // Find color
                    for (const auto& p : potentials) {
                        if (p.getId() == rule.landUseId) {
                            assignedColor = p.getColor();
                            bestPriority = rule.priority;
                            break;
                        }
                    }
                }
            }
            colors.push_back(assignedColor);
        }
        
        return colors;
    }

    /**
     * @brief Generates the Land Use ID vector for resilience analysis.
     * 
     * @param territory The physical context.
     * @param hypothesis The configuration rules.
     * @return std::vector<std::string> Land Use ID for each cell/vertex.
     * 
     * @note This method replicates the allocation logic to produce a storable vector
     *       of land use decisions, crucial for calculating spatial overlap over time.
     */
    static std::vector<std::string> generateLandUseVector(
        const Territory& territory,
        const LandUse::TerritorialHypothesis& hypothesis)
    {
        const auto& slopeLayer = territory.getSlopeLayer();
        const auto& soilLayer = territory.getSoilLayer();
        size_t count = slopeLayer.size();
        
        std::vector<std::string> landUses;
        landUses.reserve(count);
        
        if (slopeLayer.empty() || soilLayer.empty() || slopeLayer.size() != soilLayer.size()) {
            return std::vector<std::string>(count, "");
        }
        
        for (size_t i = 0; i < count; ++i) {
            float s = slopeLayer[i];
            
            std::string assignedId = ""; // Default: Unallocated
            int bestPriority = 999;
            
            // Find best matching rule
            for (const auto& rule : hypothesis.getAllocationRules()) {
                if (rule.priority >= bestPriority && bestPriority != 999) continue;

                // Check constraints
                bool fits = true;
                if (rule.parameters.count("slope_min")) {
                    if (s < std::stof(rule.parameters.at("slope_min"))) fits = false;
                }
                if (rule.parameters.count("slope_max")) {
                    if (s > std::stof(rule.parameters.at("slope_max"))) fits = false;
                }
                
                if (fits) {
                    assignedId = rule.landUseId;
                    bestPriority = rule.priority;
                }
            }
            landUses.push_back(assignedId);
        }
        
        return landUses;
    }

};

} // namespace Core::Domain::Territory
