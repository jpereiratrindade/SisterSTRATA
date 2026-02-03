#pragma once

#include "core/domain/world/WorldState.hpp"
#include <memory>

namespace Application::Ports {

/**
 * @brief Interface for the Visualization Layer.
 * Allows the Application to update the view without knowing if it's Vulkan, ASCII, or Headless.
 */
class IWorldView {
public:
    virtual ~IWorldView() = default;

    /**
     * @brief Called when the entire world state changes significantly (e.g., load file).
     */
    virtual void onWorldLoaded(const Core::Domain::WorldState& state) = 0;

    /**
     * @brief Called when a specific entity is added or updated.
     */
    virtual void onEntityUpdated(const Core::Domain::WorldEntity& entity) = 0;

    /**
     * @brief Clears the visualization.
     */
    virtual void clear() = 0;
    
    // Future expansion:
    // virtual void onCameraMoved(...)
    // virtual void onSelectionChanged(...)
};

} // namespace Application::Ports
