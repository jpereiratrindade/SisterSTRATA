#pragma once

#include <vector>
#include <string>
#include <map>

namespace Core::Domain::FourthDimension::PatchTrajectory {

/**
 * @brief Represents the state of a specific patch at a single point in time.
 * Follows Section 4 of DDD_PatchTrajectory_Analysis.
 */
struct PatchState {
    int ordinalIndex; // Time slice index
    
    // 4.1 Geometria e Forma
    float area = 0.0f;
    float perimeter = 0.0f;
    float shapeIndex = 0.0f;
    float fractalDimension = 0.0f;
    float radiusOfGyration = 0.0f;

    // 4.2 Borda e Área Núcleo
    float edgeLength = 0.0f;
    float edgeDensity = 0.0f;
    float coreArea = 0.0f;
    float coreAreaIndex = 0.0f;

    // 4.3 Contextualização Espacial (Land-Use Contrast)
    float proximityIndex = 0.0f;
    std::map<int, float> adjacencyByClass; // ClassID -> percentage or length

    // 4.4 Conectividade e Coesão
    float cohesionIndex = 0.0f;
};

} // namespace Core::Domain::FourthDimension::PatchTrajectory
