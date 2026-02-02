#pragma once

#include "core/domain/vegetation/VegetationOriginal.hpp"
#include "core/domain/vegetation/EcologicalScenario.hpp"
#include "core/value_objects/TerrainVertex.hpp"
#include <vector>
#include <cmath>
#include <algorithm> // For std::clamp
#include <limits>    // For std::numeric_limits
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include "core/domain/hydro/HydroGrid.hpp" // Required for HydroGrid definition

namespace Core::Domain::Vegetation {

/**
 * @brief Domain Service responsible for mapping declarative hypotheses to technical bases (Terrain Data).
 * 
 * This service acts as the bridge between the Core Domain (Declarative) and the 
 * Infrastructure/World (Technical Bases). It computes the spatial extent of a hypothesis.
 */
class VegetationMappingService {
public:
    /**
     * @brief Result of a mapping operation.
     */
    struct MappingResult {
        size_t totalVertices;
        size_t matchVertices;
        float coveragePercentage;
        std::vector<bool> coverageMask; // New: Per-vertex coverage status
    };

    /**
     * @brief Calculates the potential coverage of a vegetation hypothesis on the given terrain data.
     * 
     * @param hypothesis The vegetation hypothesis containing criteria (slope, drainage distance).
     * @param vertices The raw terrain vertices (Technical Base).
     * @param hydro The hydrological grid data (must be valid if maxDistanceToDrainage is set).
     * @param gridSpacing The spatial resolution of the grid (in meters) to convert cell distance to real distance.
     * @return MappingResult Statistics about the potential coverage.
     */
    static MappingResult calculatePotentialCoverage(
        const VegetationOriginal& hypothesis,
        const std::vector<Core::ValueObjects::TerrainVertex>& vertices,
        const Core::Domain::Hydro::HydroGrid& hydro,
        float gridSpacing
    ) {
        if (vertices.empty()) return {0, 0, 0.0f, {}};

        size_t matches = 0;
        std::vector<bool> coverageMask(vertices.size(), false); // Init mask
        const auto& conditions = hypothesis.getConditions();

        // Distance Map Preparation (Lazy Evaluation)
        std::vector<float> drainageDistances;
        bool checkDrainage = conditions.maxDistanceToDrainage.has_value() && 
                             hydro.isValid() && 
                             hydro.flowAccumulationCells.size() == vertices.size();

        if (checkDrainage) {
            // BFS to calculate Euclidean-ish distance from "Drainage" cells (Accum > 100)
            const int STREAM_THRESHOLD = 500; // Conservative threshold for "River"
            
            drainageDistances.assign(vertices.size(), std::numeric_limits<float>::max());
            std::vector<int> q;
            q.reserve(vertices.size() / 10);

            for (size_t i = 0; i < hydro.flowAccumulationCells.size(); ++i) {
                if (hydro.flowAccumulationCells[i] > STREAM_THRESHOLD) {
                    drainageDistances[i] = 0.0f;
                    q.push_back((int)i);
                }
            }

            int w = hydro.width;
            int h = hydro.height;
            size_t head = 0;
            
            // 4-neighbor BFS
            const int dx[] = {0, 0, 1, -1};
            const int dy[] = {1, -1, 0, 0};
            
            while(head < q.size()) {
                int curr = q[head++];
                int cx = curr % w;
                int cy = curr / w;
                float d = drainageDistances[curr];
                
                if (d * gridSpacing > conditions.maxDistanceToDrainage.value()) continue;

                for(int k=0; k<4; ++k) {
                    int nx = cx + dx[k];
                    int ny = cy + dy[k];
                    if (nx >=0 && nx < w && ny >=0 && ny < h) {
                        int nidx = ny * w + nx;
                        if (drainageDistances[nidx] > d + 1.0f) {
                            drainageDistances[nidx] = d + 1.0f;
                            q.push_back(nidx);
                        }
                    }
                }
            }
        }

        for (size_t i = 0; i < vertices.size(); ++i) {
            const auto& v = vertices[i];
            
            // Slope Check
            float slopeRad = std::acos(std::clamp(v.normal.z, -1.0f, 1.0f)); 
            float slopeDeg = glm::degrees(slopeRad);

            bool fit = true;
            if (conditions.minSlope.has_value() && slopeDeg < conditions.minSlope.value()) fit = false;
            if (conditions.maxSlope.has_value() && slopeDeg > conditions.maxSlope.value()) fit = false;
            
            // Drainage Check
            if (fit && checkDrainage) {
                float dist = drainageDistances[i];
                if (dist == std::numeric_limits<float>::max()) {
                     fit = false;
                } else {
                     float realDist = dist * gridSpacing;
                     if (realDist > conditions.maxDistanceToDrainage.value()) fit = false;
                }
            }

            if (fit) {
                matches++;
                coverageMask[i] = true;
            }
        }

        return {
            vertices.size(),
            matches,
            (static_cast<float>(matches) / vertices.size()) * 100.0f,
            coverageMask
        };
    }

    struct ScenarioStats {
        size_t realizedVertices;
        float realizedPercentage;
    };

    struct ScenarioResult {
        std::vector<int> classification; // -1=None, >=0 Hypothesis Index
        std::vector<int> semanticCodes;  // Semantic VegetationCode for the winner
        std::vector<ScenarioStats> stats;
    };

    static std::vector<float> calculateDrainageMap(const std::vector<Core::ValueObjects::TerrainVertex>& vertices, const Core::Domain::Hydro::HydroGrid& hydro) {
         if (!hydro.isValid() || hydro.flowAccumulationCells.size() != vertices.size()) return {};
         
         const int STREAM_THRESHOLD = 500;
         std::vector<float> drainageDistances(vertices.size(), std::numeric_limits<float>::max());
         std::vector<int> q;
         q.reserve(vertices.size() / 10);

         for (size_t i = 0; i < hydro.flowAccumulationCells.size(); ++i) {
             if (hydro.flowAccumulationCells[i] > STREAM_THRESHOLD) {
                 drainageDistances[i] = 0.0f;
                 q.push_back((int)i);
             }
         }

         int w = hydro.width;
         int h = hydro.height;
         size_t head = 0;
         const int dx[] = {0, 0, 1, -1};
         const int dy[] = {1, -1, 0, 0};
         
         while(head < q.size()) {
             int curr = q[head++];
             int cx = curr % w;
             int cy = curr / w;
             float d = drainageDistances[curr];
             
             for(int k=0; k<4; ++k) {
                 int nx = cx + dx[k];
                 int ny = cy + dy[k];
                 if (nx >=0 && nx < w && ny >=0 && ny < h) {
                     int nidx = ny * w + nx;
                     if (drainageDistances[nidx] > d + 1.0f) {
                         drainageDistances[nidx] = d + 1.0f;
                         q.push_back(nidx);
                     }
                 }
             }
         }
         return drainageDistances;
    }

    // Updated individual calculation to use helper if needed, or keeping it standalone for now? 
    // Keeping standalone logic inside calculatePotentialCoverage for backward compat/speed if needed, OR refactor.
    // To minimize risk, I will implement calculateScenario using this new helper, and leave calculatePotentialCoverage as is (or refactor it to use helper).
    // Refactoring is cleaner.
    
    /**
     * @brief Calculates a global scenario overlay where multiple scenarios compete.
     * Priority is given based on position in the scenarios vector (Top wins).
     * 
     * @param scenarios List of scenarios to overlay.
     * @param vertices Terrain vertices to classify.
     * @param hydro Hydrological data for drainage distance calculation.
     * @param gridSpacing Distance in meters between adjacent grid cells.
     * @return ScenarioResult A classification map of scenario indices and semantic codes.
     */
    static ScenarioResult calculateScenario(
        const std::vector<EcologicalScenario>& scenarios,
        const std::vector<Core::ValueObjects::TerrainVertex>& vertices,
        const Core::Domain::Hydro::HydroGrid& hydro,
        float gridSpacing
    ) {
         if (vertices.empty()) return {{}, {}, {}};
         
         // Pre-calculate drainage map ONCE
         std::vector<float> drainageMap = calculateDrainageMap(vertices, hydro);
         bool hasDrainage = !drainageMap.empty();

          std::vector<int> classification(vertices.size(), -1);
          std::vector<int> semanticCodes(vertices.size(), -1);
          std::vector<ScenarioStats> stats(scenarios.size(), {0, 0.0f});

          for (size_t i = 0; i < vertices.size(); ++i) {
              const auto& v = vertices[i];
              float slopeRad = std::acos(std::clamp(v.normal.z, -1.0f, 1.0f)); 
              float slopeDeg = glm::degrees(slopeRad);

              // Priority: First scenario in the system list wins for global overlay
              for (size_t sIdx = 0; sIdx < scenarios.size(); ++sIdx) {
                  const auto& scenario = scenarios[sIdx];
                  bool match = false;

                  for (const auto& h : scenario.getComponents()) {
                      const auto& cond = h.getConditions();
                      bool fit = true;
                      if (cond.minSlope.has_value() && slopeDeg < cond.minSlope.value()) fit = false;
                      if (cond.maxSlope.has_value() && slopeDeg > cond.maxSlope.value()) fit = false;
                      
                      if (fit && cond.maxDistanceToDrainage.has_value() && hasDrainage) {
                          float dist = drainageMap[i];
                          if (dist * gridSpacing > cond.maxDistanceToDrainage.value()) fit = false;
                      }

                      if (fit) {
                          classification[i] = (int)sIdx; // For global overlay, we store scenario index
                          semanticCodes[i] = static_cast<int>(h.getType().getCode());
                          match = true;
                          break; 
                      }
                  }
                  if (match) {
                      stats[sIdx].realizedVertices++;
                      break;
                  }
              }
          }

         for(auto& s : stats) {
             s.realizedPercentage = (static_cast<float>(s.realizedVertices) / vertices.size()) * 100.0f;
         }

          return {classification, semanticCodes, stats};
     }

    /**
     * @brief Resolves a specific EcologicalScenario into a semantic classification map.
     * Components within the scenario compete (First that fits wins).
     * 
     * @param scenario The scenario to resolve.
     * @param vertices Terrain vertices to classify.
     * @param hydro Hydrological data for drainage distance calculation.
     * @param gridSpacing Distance in meters between adjacent grid cells.
     * @return std::vector<int> A map containing semantic VegetationCode values.
     */
    static std::vector<int> resolveScenarioToCodes(
        const EcologicalScenario& scenario,
        const std::vector<Core::ValueObjects::TerrainVertex>& vertices,
        const Core::Domain::Hydro::HydroGrid& hydro,
        float gridSpacing
    ) {
        if (vertices.empty()) return {};

        std::vector<float> drainageMap = calculateDrainageMap(vertices, hydro);
        bool hasDrainage = !drainageMap.empty();

        std::vector<int> classification(vertices.size(), -1);

        for (size_t i = 0; i < vertices.size(); ++i) {
            const auto& v = vertices[i];
            float slopeRad = std::acos(std::clamp(v.normal.z, -1.0f, 1.0f)); 
            float slopeDeg = glm::degrees(slopeRad);

            // Priority: Components within the same scenario (vector) compete
            for (const auto& h : scenario.getComponents()) {
                const auto& cond = h.getConditions();
                bool fit = true;
                if (cond.minSlope.has_value() && slopeDeg < cond.minSlope.value()) fit = false;
                if (cond.maxSlope.has_value() && slopeDeg > cond.maxSlope.value()) fit = false;
                
                if (fit && cond.maxDistanceToDrainage.has_value() && hasDrainage) {
                    float dist = drainageMap[i];
                    if (dist * gridSpacing > cond.maxDistanceToDrainage.value()) fit = false;
                }

                if (fit) {
                    classification[i] = static_cast<int>(h.getType().getCode());
                    break; // First component that fits wins
                }
            }
        }
        return classification;
    }
};

} // namespace Core::Domain::Vegetation
