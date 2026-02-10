#pragma once

namespace UI::Panels {

/**
 * @brief Application-wide settings panel.
 */
class SettingsPanel {
public:
    void setMultiViewportControls(bool* requested, const bool* supported, const bool* active);
    /**
     * @brief Render the settings panel.
     */
    void draw(bool* open);

private:
    bool* multiViewportRequested_ = nullptr;
    const bool* multiViewportSupported_ = nullptr;
    const bool* multiViewportActive_ = nullptr;
};

} // namespace UI::Panels
