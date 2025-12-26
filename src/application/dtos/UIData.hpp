#pragma once

#include <string>

namespace Application::DTO {

struct UIData {
    // System Performance
    float framerate = 0.0f;
    float frameTimeMs = 0.0f;

    // Session / Domain State
    std::string startMessage;
    
    // Application State
    bool pendingChanges = false;
};

} // namespace Application::DTO
