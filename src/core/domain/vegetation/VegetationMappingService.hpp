#pragma once

#include "core/domain/vegetation/VegetationOriginal.hpp"
#include "world3d/rendering/Vertex.hpp"
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
        const std::vector<World3D::Rendering::Vertex>& vertices,
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
        std::vector<ScenarioStats> stats;
    };

    static std::vector<float> calculateDrainageMap(const std::vector<World3D::Rendering::Vertex>& vertices, const Core::Domain::Hydro::HydroGrid& hydro) {
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
    
    // ... calculatePotentialCoverage ... (Leaving as is to avoid large diff, or refactoring?)
    // User wants "Scenario". I'll add "calculateScenario".
    
    static ScenarioResult calculateScenario(
        const std::vector<VegetationOriginal>& hypotheses,
        const std::vector<World3D::Rendering::Vertex>& vertices,
        const Core::Domain::Hydro::HydroGrid& hydro,
        float gridSpacing
    ) {
         if (vertices.empty()) return {{}, {}};
         
         // Pre-calculate drainage map ONCE
         std::vector<float> drainageMap = calculateDrainageMap(vertices, hydro);
         bool hasDrainage = !drainageMap.empty();

         std::vector<int> classification(vertices.size(), -1);
         std::vector<ScenarioStats> stats(hypotheses.size(), {0, 0.0f});

         for (size_t i = 0; i < vertices.size(); ++i) {
             const auto& v = vertices[i];
             
             // Slope Logic
             float slopeRad = std::acos(std::clamp(v.normal.z, -1.0f, 1.0f)); 
             float slopeDeg = glm::degrees(slopeRad);

             // Find first match (Priority: Index 0 wins)
             for (size_t hIdx = 0; hIdx < hypotheses.size(); ++hIdx) {
                 const auto& h = hypotheses[hIdx];
                 const auto& cond = h.getConditions();
                 
                 bool fit = true;
                 if (cond.minSlope.has_value() && slopeDeg < cond.minSlope.value()) fit = false;
                 if (cond.maxSlope.has_value() && slopeDeg > cond.maxSlope.value()) fit = false;
                 
                 if (fit && cond.maxDistanceToDrainage.has_value()) {
                     if (!hasDrainage) {
                         // Missing dependency -> Fail or Ignore?
                         // In individual mode we ignore? Or fail?
                         // "Safe" approach: if criteria exists but data missing, fail match.
                         fit = false; 
                     } else {
                         float dist = drainageMap[i];
                         if (dist * gridSpacing > cond.maxDistanceToDrainage.value()) fit = false;
                     }
                 }
                 
                 if (fit) {
                     classification[i] = (int)hIdx;
                     stats[hIdx].realizedVertices++;
                     break; // Claimed!
                 }
             }
         }

         for(auto& s : stats) {
             s.realizedPercentage = (static_cast<float>(s.realizedVertices) / vertices.size()) * 100.0f;
         }

         return {classification, stats};
    }
};

} // namespace Core::Domain::Vegetation
