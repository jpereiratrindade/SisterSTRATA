#pragma once
#include "application/dtos/UIData.hpp"

namespace UI::Panels {

class WelcomePanel {
public:
    void draw(bool* open, const Application::DTO::UIData& data);
};

} // namespace UI::Panels
