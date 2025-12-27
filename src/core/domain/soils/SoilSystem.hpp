#pragma once
#include <vector>
#include "world3d/rendering/Vertex.hpp"
#include "core/domain/soils/Scorpan.hpp"
#include "core/domain/soils/SiBCS.hpp"

namespace Core::Domain::Soils {

class SoilSystem {
public:
    /**
     * @brief Processes the terrain vertices and applies soil classification colors based on SCORPAN parameters.
     * 
     * @param vertices The vertex buffer to modify.
     * @param params The SCORPAN global parameters (Climate, Organisms, etc.).
     * @param visualizationLevel The SiBCS level to visualize (1=Order, 2=Suborder, 3=GreatGroup...)
     */
    static void process(std::vector<World3D::Rendering::Vertex>& vertices, const ScorpanParams& params, int visualizationLevel = 1);

private:
    static SiBCSClassification predict(const ScorpanParams& global, float slopeDeg, float elevation, float relElevation);
};

} // namespace Core::Domain::Soils
