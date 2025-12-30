#pragma once

#include <string>

namespace UI::Panels {

/**
 * @brief Panel for Procedural Terrain Generation and Export.
 */
class TerrainGeneratorPanel {
public:
    /**
     * @brief Default constructor. Initializes default generation parameters.
     */
    TerrainGeneratorPanel();

    /**
     * @brief Renders the Terrain Generator UI panel.
     * @param open Pointer to a boolean flag controlling the panel's visibility.
     */
    void draw(bool* open);

private:
    // UI State
    int width_ = 256;
    int height_ = 256;
    float spacing_ = 2.0f;
    int selectedType_ = 0; // 0=Flat, 1=Hills, etc.
    
    char filenameBuffer_[256] = "terrain_export.csv";
    bool autoLoad_ = true;

    // Generation State
    bool isGenerating_ = false;
    float progress_ = 0.0f;
    std::string statusMessage_;
};

} // namespace UI::Panels
