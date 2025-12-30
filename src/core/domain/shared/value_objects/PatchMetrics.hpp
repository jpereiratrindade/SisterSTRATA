#pragma once

namespace Core::Domain::Shared::ValueObjects {

/**
 * @brief Composite value object representing the spatial structural metrics of a Patch.
 * 
 * These metrics are used to evaluate spatial patterns such as fragmentation, 
 * shape complexity, and connectivity, which feed into the Territorial Coherence evaluation.
 */
struct PatchMetrics {
    /**
     * @brief Area of the patch (in grid cells or square meters).
     */
    float area;

    /**
     * @brief Shape Index (SI). 
     * Ratio of perimeter to area, normalized to a circle/square.
     * Higher values indicate more complex, convoluted shapes (often less coherent for anthropogenic uses, 
     * but potentially high value for ecological edges).
     */
    float shapeIndex;

    /**
     * @brief Fractal Dimension.
     * Indicates the complexity of the patch boundary structure.
     */
    float fractalDimension;

    /**
     * @brief Connectivity Index.
     * Measures how well-connected this patch is to neighbors of the same type.
     */
    float connectivity;

    /**
     * @brief Default constructor initializing to zero.
     */
    PatchMetrics() : area(0), shapeIndex(0), fractalDimension(0), connectivity(0) {}

    /**
     * @brief Full constructor.
     */
    PatchMetrics(float a, float si, float fd, float c) 
        : area(a), shapeIndex(si), fractalDimension(fd), connectivity(c) {}
};

} // namespace Core::Domain::Shared::ValueObjects
