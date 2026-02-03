#include "ui/views/Hybrid2DView.hpp"
#include <iostream>

namespace UI::Views {

Hybrid2DView::Hybrid2DView() {
    std::cout << "[Hybrid2DView] Created (Category D - CPU Mode)." << std::endl;
}

Hybrid2DView::~Hybrid2DView() {
    if (renderer_) {
        SDL_DestroyRenderer(renderer_);
    }
}

void Hybrid2DView::init(SDL_Window* window) {
    window_ = window;
    // Force Software Renderer for architectural compliance (CPU only)
    renderer_ = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if (!renderer_) {
        std::cerr << "[Hybrid2DView] Failed to create software renderer: " << SDL_GetError() << std::endl;
    }
    if (!renderer_) {
        std::cerr << "[Hybrid2DView] Failed to create software renderer: " << SDL_GetError() << std::endl;
    }
    
    // High-DPI Fix: Force Logical Size to match Window
    // This tells SDL to scale our 1920x1080 buffer to whatever 4K/Retina surface exists.
    SDL_GetWindowSize(window, &width_, &height_);
    SDL_RenderSetLogicalSize(renderer_, width_, height_);
    
    // Center view
    offsetX_ = 0.0f;
    offsetY_ = 0.0f;
    scale_ = 5.0f;
}

void Hybrid2DView::handleEvent(const SDL_Event& event) {
    if (event.type == SDL_KEYDOWN) {
        float step = 50.0f / scale_;
        switch (event.key.keysym.sym) {
            case SDLK_w: case SDLK_UP:    offsetY_ += step; updateCachedPoints(); break;
            case SDLK_s: case SDLK_DOWN:  offsetY_ -= step; updateCachedPoints(); break;
            case SDLK_a: case SDLK_LEFT:  offsetX_ += step; updateCachedPoints(); break;
            case SDLK_d: case SDLK_RIGHT: offsetX_ -= step; updateCachedPoints(); break;
            case SDLK_TAB: cycleView(); break;
            case SDLK_f: fitToScreen(); break;
        }
    } else if (event.type == SDL_MOUSEWHEEL) {
        if (event.wheel.y > 0) scale_ *= 1.2f;
        else if (event.wheel.y < 0) scale_ /= 1.2f;
        updateCachedPoints();
    } else if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED) {
        // Update Logical Size on Resize
        int newW = event.window.data1;
        int newH = event.window.data2;
        if (renderer_) SDL_RenderSetLogicalSize(renderer_, newW, newH);
        
        width_ = newW;
        height_ = newH;
        fitToScreen();
    }
}

void Hybrid2DView::onWorldLoaded(const Core::Domain::WorldState& state) {
    lastState_ = &state;
    fitToScreen(); // Auto-fit on load
    std::cout << "[Hybrid2DView] World loaded. Cached " << cachedPoints_.size() << " points." << std::endl;
}

void Hybrid2DView::fitToScreen() {
    if (!lastState_ || lastState_->entities.empty()) return;
    
    // Always sync with renderer size for High-DPI correctness
    if (renderer_) {
        SDL_GetRendererOutputSize(renderer_, &width_, &height_);
    } else if (window_) {
        SDL_GetWindowSize(window_, &width_, &height_);
    }

    // Calculate Bounds
    float minX = 1e9f, maxX = -1e9f;
    float minY = 1e9f, maxY = -1e9f;

    for (const auto& entity : lastState_->entities) {
        for (const auto& p : entity.points) {
            float px = 0, py = 0;
            switch(currentView_) {
                case ViewPlane::XY: px = p.x; py = p.y; break;
                case ViewPlane::XZ: px = p.x; py = p.z; break;
                case ViewPlane::YZ: px = p.y; py = p.z; break;
            }
            if (px < minX) minX = px;
            if (px > maxX) maxX = px;
            if (py < minY) minY = py;
            if (py > maxY) maxY = py;
        }
    }

    float dataW = maxX - minX;
    float dataH = maxY - minY;
    
    if (dataW < 0.001f) dataW = 1.0f;
    if (dataH < 0.001f) dataH = 1.0f;

    // Calculate Scale to fit with tight margin (95%)
    float scaleX = (width_ * 0.95f) / dataW;
    float scaleY = (height_ * 0.95f) / dataH;
    scale_ = (scaleX < scaleY) ? scaleX : scaleY;

    // Center Logic
    offsetX_ = -(minX + dataW / 2.0f);
    offsetY_ = -(minY + dataH / 2.0f);

    std::cout << "[Hybrid2DView] Auto-Fit: Screen=" << width_ << "x" << height_ 
              << " Data=" << dataW << "x" << dataH 
              << " Scale=" << scale_ << " Margin=" << (width_ - dataW*scale_) << "x" << (height_ - dataH*scale_)
              << std::endl;

    updateCachedPoints();
}

void Hybrid2DView::updateCachedPoints() {
    if (!lastState_) return;

    std::lock_guard<std::mutex> lock(dataMutex_);
    cachedPoints_.clear();
    
    int centerX = width_ / 2;
    int centerY = height_ / 2;
    
    for (const auto& entity : lastState_->entities) {
        for (size_t i = 0; i < entity.points.size(); ++i) {
            const auto& p = entity.points[i];
            
            // Projection based on ViewPlane
            float px = 0, py = 0;
            switch(currentView_) {
                case ViewPlane::XY: px = p.x; py = p.y; break; // Top-Down (if Z=up) or Front (if Y=up)
                case ViewPlane::XZ: px = p.x; py = p.z; break; // Top-Down (if Y=up) or Side (if Z=up)
                case ViewPlane::YZ: px = p.y; py = p.z; break; // Side
            }

            int sx = centerX + (int)((px + offsetX_) * scale_);
            int sy = centerY + (int)((py + offsetY_) * scale_);
            
            // Culling (optimization for large clouds in 2D)
            if (sx < 0 || sx >= width_ || sy < 0 || sy >= height_) continue;

            // Color mapping
            uint8_t r = 255, g = 255, b = 255;
            if (i < entity.colors.size()) {
                r = (uint8_t)(entity.colors[i].x * 255);
                g = (uint8_t)(entity.colors[i].y * 255);
                b = (uint8_t)(entity.colors[i].z * 255);
            }
            
            cachedPoints_.push_back({sx, sy, r, g, b});
        }
    }
}

void Hybrid2DView::onEntityUpdated(const Core::Domain::WorldEntity& entity) {
    // For now re-trigger full update if state exists
    updateCachedPoints();
}

void Hybrid2DView::clear() {
    std::lock_guard<std::mutex> lock(dataMutex_);
    cachedPoints_.clear();
}

void Hybrid2DView::render() {
    if (!renderer_) return;

    // Clear background (Dark Grey)
    SDL_SetRenderDrawColor(renderer_, 30, 30, 30, 255);
    SDL_RenderClear(renderer_);

    // Draw Points
    {
        std::lock_guard<std::mutex> lock(dataMutex_);
        for (const auto& p : cachedPoints_) {
            SDL_SetRenderDrawColor(renderer_, p.r, p.g, p.b, 255);
            SDL_RenderDrawPoint(renderer_, p.x, p.y);
        }
    }

    SDL_RenderDrawLine(renderer_, width_/2, 0, width_/2, height_); // Z-Axis
    SDL_RenderDrawLine(renderer_, 0, height_/2, width_, height_/2); // X-Axis
}

} // namespace UI::Views
