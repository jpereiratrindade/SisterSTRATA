#include "world3d/Engine.hpp"
#include "world3d/adapters/TerrainVertexAdapter.hpp"
#include "core/domain/soils/SoilSystem.hpp"
#include <iostream>

namespace World3D {

void Engine::applySoilSimulation(const ::Core::Domain::Soils::ScorpanParams& params, int visualizationLevel, const ::Core::Domain::Soils::SiBCSFilter& filter) {
    if (!activeVertices_ || !activeVertexBuffer_) {
        std::cerr << "[Engine] No active mesh to simulate soils." << std::endl;
        return;
    }

    auto terrainVertices = World3D::Adapters::toTerrainVertices(*activeVertices_);
    Core::Domain::Soils::SoilSystem::process(terrainVertices, params, visualizationLevel, filter);
    World3D::Adapters::applyTerrainColors(terrainVertices, *activeVertices_);

    hydroVisMode_ = HydroVisMode::None;
    baseColorsValid_ = false;

    vk::DeviceSize size = sizeof(Rendering::Vertex) * activeVertices_->size();
    Rendering::Buffer staging(*context_, size, vk::BufferUsageFlagBits::eTransferSrc,
                              vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    staging.copyTo(activeVertices_->data(), size);

    renderer_->copyBuffer(staging.getHandle(), activeVertexBuffer_->getHandle(), size);
}

} // namespace World3D
