#pragma once
#include <string>
#include <glm/glm.hpp>

namespace Core::Domain::Soils {

enum class SiBCSClass {
    Latossolo,   // Deep, highly weathered (Old, Flat)
    Argissolo,   // Clay accumulation (Moderate Slope/Age)
    Neossolo,    // Young, shallow (Steep, Young)
    Cambissolo,  // Incipient B horizon (Transitional)
    Gleissolo,   // Hydromorphic (Low relief, High water)
    Planossolo,  // Abrupt transition (Low relief)
    Espodossolo, // Podzolization (Sandy, organic accumulation)
    Unknown
};

struct SiBCSHelper {
    static std::string getName(SiBCSClass type) {
        switch(type) {
            case SiBCSClass::Latossolo: return "Latossolo (L)";
            case SiBCSClass::Argissolo: return "Argissolo (P)";
            case SiBCSClass::Neossolo: return "Neossolo (R)";
            case SiBCSClass::Cambissolo: return "Cambissolo (C)";
            case SiBCSClass::Gleissolo: return "Gleissolo (G)";
            case SiBCSClass::Planossolo: return "Planossolo (S)";
            case SiBCSClass::Espodossolo: return "Espodossolo (E)";
            default: return "Unknown";
        }
    }

    static glm::vec3 getColor(SiBCSClass type) {
        switch(type) {
            case SiBCSClass::Latossolo: return glm::vec3(0.6f, 0.2f, 0.2f); // Dark Red
            case SiBCSClass::Argissolo: return glm::vec3(0.8f, 0.4f, 0.2f); // Orange-Red
            case SiBCSClass::Neossolo: return glm::vec3(0.5f, 0.5f, 0.5f);  // Grey (Rock-like)
            case SiBCSClass::Cambissolo: return glm::vec3(0.7f, 0.6f, 0.3f); // Brownish Yellow
            case SiBCSClass::Gleissolo: return glm::vec3(0.3f, 0.4f, 0.5f); // Grey-Blue (Waterlogged)
            case SiBCSClass::Planossolo: return glm::vec3(0.8f, 0.7f, 0.6f); // Pale
            case SiBCSClass::Espodossolo: return glm::vec3(0.2f, 0.2f, 0.2f); // Dark/Black (Organic/Sandy)
            default: return glm::vec3(1.0f); // White
        }
    }
};

} // namespace Core::Domain::Soils
