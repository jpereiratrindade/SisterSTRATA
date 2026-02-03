#pragma once

#include "application/ports/IWorldView.hpp"
#include <SDL2/SDL.h>
#include <vector>
#include <mutex>
#include <string>

namespace UI::Views {

/**
 * @brief Hybrid CPU-based visualization (Category D).
 * Renders orthogonal projections of the 3D WorldState using pure CPU rasterization.
 * Used when Vulkan is unavailable or for scientific validation.
 */
class Hybrid2DView : public Application::Ports::IWorldView {
public:
    Hybrid2DView();
    ~Hybrid2DView();

    void init(SDL_Window* window);

    // IWorldView Implementation
    void onWorldLoaded(const Core::Domain::WorldState& state) override;
    void onEntityUpdated(const Core::Domain::WorldEntity& entity) override;
    void clear() override;

    // View Modes
    enum class ViewPlane { XY, XZ, YZ };
    void cycleView() {
        currentView_ = static_cast<ViewPlane>((static_cast<int>(currentView_) + 1) % 3);
        fitToScreen(); // Re-fit on view change
    }
    std::string getViewName() const {
        switch(currentView_) {
            case ViewPlane::XY: return "Top (XY)";
            case ViewPlane::XZ: return "Side (XZ)";
            case ViewPlane::YZ: return "Side (YZ)";
            default: return "Unknown";
        }
    }

    void fitToScreen(); // New: Auto-fit content

    // Rendering
    void render(); // Called by Application loop
    SDL_Renderer* getRenderer() const { return renderer_; }

    void handleEvent(const SDL_Event& event); // New: for navigation
    
private:
    void drawTopDownView();
    void drawSideView();
    void updateCachedPoints(); // Move flattening logic here

    struct Point2D {
        int x, y;
        uint8_t r, g, b;
    };

    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    
    // Perspective/Navigation
    ViewPlane currentView_ = ViewPlane::XY; // Default to XY (Common for GIS/CSV)
    float offsetX_ = 0.0f;
    float offsetY_ = 0.0f;
    float scale_ = 5.0f;

    const Core::Domain::WorldState* lastState_ = nullptr;
    
    // Local copy of renderable data
    std::vector<Point2D> cachedPoints_;
    std::mutex dataMutex_;

    int width_ = 800;
    int height_ = 600;
};

} // namespace UI::Views
