#pragma once
#include <string>
#include <glm/glm.hpp>

namespace Core::Domain::Soils {

// 1. Order (13 Classes)
enum class SiBCSOrder {
    Latossolo,   // L
    Argissolo,   // P
    Neossolo,    // R
    Cambissolo,  // C
    Gleissolo,   // G
    Planossolo,  // S
    Espodossolo, // E
    Vertissolo,  // V
    Nitossolo,   // N
    Chernossolo, // M
    Luvissolo,   // T
    Plintossolo, // F
    Organossolo, // O
    Unknown
};

// 2. Suborder ( simplified for simulation )
enum class SiBCSSuborder {
    Vermelho,    // Red (Iron rich)
    Amarelo,     // Yellow (Goethite)
    VermelhoAmarelo, // Red-Yellow
    Bruno,       // Brown
    Haplic,      // Simple
    Litoc,       // Stony
    Gleico,      // Hydra
    Tiptico,     // Typical
    None
};

// 3. Great Group
enum class SiBCSGreatGroup {
    Distrofico, // Low saturation (< 50%)
    Eutrofico,  // High saturation (>= 50%)
    Alico,      // High Aluminum
    Humico,     // High Organic Carbon
    None
};

// 4. Subgroup (Simplified)
enum class SiBCSSubgroup {
    Tipico,
    Latossolico,
    Argissolico,
    None
};

// 5. Family (Placeholder)
enum class SiBCSFamily {
    TexturaArgilosa,
    TexturaMedia,
    TexturaArenosa,
    None
};

// 6. Series (Placeholder)
enum class SiBCSSeries {
    Generica,
    None
};

// Full Classification Struct
// Full Classification Struct (Vector DTO)
struct SiBCSClassification {
    SiBCSOrder order = SiBCSOrder::Unknown;
    SiBCSSuborder suborder = SiBCSSuborder::None;
    SiBCSGreatGroup greatGroup = SiBCSGreatGroup::None;
    SiBCSSubgroup subgroup = SiBCSSubgroup::None;
    SiBCSFamily family = SiBCSFamily::None;
    SiBCSSeries series = SiBCSSeries::None;

    bool operator==(const SiBCSClassification& other) const {
        return order == other.order &&
               suborder == other.suborder &&
               greatGroup == other.greatGroup &&
               subgroup == other.subgroup &&
               family == other.family &&
               series == other.series;
    }
};

// Visualization Helper
struct SiBCSHelper {
    // Basic Name Lookups
    static std::string getBaseName(SiBCSOrder type) {
        switch(type) {
            case SiBCSOrder::Latossolo: return "Latossolo";
            case SiBCSOrder::Argissolo: return "Argissolo";
            case SiBCSOrder::Neossolo: return "Neossolo";
            case SiBCSOrder::Cambissolo: return "Cambissolo";
            case SiBCSOrder::Gleissolo: return "Gleissolo";
            case SiBCSOrder::Planossolo: return "Planossolo";
            case SiBCSOrder::Espodossolo: return "Espodossolo";
            case SiBCSOrder::Vertissolo: return "Vertissolo";
            case SiBCSOrder::Nitossolo: return "Nitossolo";
            case SiBCSOrder::Organossolo: return "Organossolo";
            default: return "Outro";
        }
    }
    
    static std::string getBaseName(SiBCSSuborder type) {
        switch(type) {
            case SiBCSSuborder::Vermelho: return "Vermelho";
            case SiBCSSuborder::Amarelo: return "Amarelo";
            case SiBCSSuborder::VermelhoAmarelo: return "Vermelho-Amarelo";
            case SiBCSSuborder::Bruno: return "Bruno";
            case SiBCSSuborder::Litoc: return "Litólico";
            case SiBCSSuborder::Gleico: return "Gleico"; // Usually combined
            case SiBCSSuborder::Haplic: return "Háplico";
            default: return "";
        }
    }

    static std::string getBaseName(SiBCSGreatGroup type) {
        switch(type) {
            case SiBCSGreatGroup::Distrofico: return "Distrófico";
            case SiBCSGreatGroup::Eutrofico: return "Eutrófico";
            case SiBCSGreatGroup::Alico: return "Álico";
            case SiBCSGreatGroup::Humico: return "Húmico";
            default: return "";
        }
    }

    // Cumulative Name Generation
    static std::string getName(const SiBCSClassification& soil, int level) {
        std::string name = getBaseName(soil.order);
        if (level >= 2 && soil.suborder != SiBCSSuborder::None) {
            name += " " + getBaseName(soil.suborder);
        }
        if (level >= 3 && soil.greatGroup != SiBCSGreatGroup::None) {
            name += " " + getBaseName(soil.greatGroup);
        }
        // ... (extend for levels 4-6)
        if (level >= 4 && soil.subgroup != SiBCSSubgroup::None) {
             name += " típico"; // Placeholder for enum string
        }
        return name;
    }

    // Context-Aware Colors
    static glm::vec3 getColor(const SiBCSClassification& soil, int level) {
        // Level 1: Order Base Colors
        if (level == 1) {
            switch(soil.order) {
                case SiBCSOrder::Latossolo: return glm::vec3(0.75f, 0.15f, 0.15f); // Deep Red
                case SiBCSOrder::Argissolo: return glm::vec3(0.9f, 0.5f, 0.2f); // Orange
                case SiBCSOrder::Neossolo: return glm::vec3(0.9f, 0.9f, 0.7f); // Pale
                case SiBCSOrder::Cambissolo: return glm::vec3(0.6f, 0.4f, 0.2f); // Brown
                case SiBCSOrder::Gleissolo: return glm::vec3(0.4f, 0.6f, 0.7f); // Blue-Grey
                default: return glm::vec3(0.5f);
            }
        }

        // Level 2+: Suborder Specifics
        if (level >= 2) {
            // Latossolos
            if (soil.order == SiBCSOrder::Latossolo) {
                switch(soil.suborder) {
                    case SiBCSSuborder::Vermelho: return glm::vec3(0.6f, 0.1f, 0.1f); // Darker Red
                    case SiBCSSuborder::Amarelo: return glm::vec3(0.9f, 0.8f, 0.1f); // Yellow
                    case SiBCSSuborder::VermelhoAmarelo: return glm::vec3(0.9f, 0.4f, 0.1f); // Orange
                    default: return glm::vec3(0.75f, 0.15f, 0.15f); // Base
                }
            }
            // Argissolos
            if (soil.order == SiBCSOrder::Argissolo) {
                switch(soil.suborder) {
                    case SiBCSSuborder::Vermelho: return glm::vec3(0.7f, 0.2f, 0.1f); // Reddish
                    case SiBCSSuborder::Amarelo: return glm::vec3(0.95f, 0.7f, 0.2f); // Yellowish
                    default: return glm::vec3(0.9f, 0.5f, 0.2f); // Base
                }
            }
             // Gleissolos
            if (soil.order == SiBCSOrder::Gleissolo) {
                 return glm::vec3(0.3f, 0.5f, 0.6f); // Bluish
            }
            // Neossolos
            if (soil.order == SiBCSOrder::Neossolo) {
                 if (soil.suborder == SiBCSSuborder::Litoc) return glm::vec3(0.6f, 0.6f, 0.6f); // Grey
                 return glm::vec3(0.9f, 0.9f, 0.8f); // Pale
            }
        }

        // Level 3+: Fertility Modifiers (Subtle tint)
        glm::vec3 c = getColor(soil, 2); // Base on Suborder
        if (level >= 3) {
            if (soil.greatGroup == SiBCSGreatGroup::Eutrofico) {
                c *= 1.1f; // Brighter (Fertile)
            } else if (soil.greatGroup == SiBCSGreatGroup::Distrofico) {
                c *= 0.9f; // Darker (Leached)
            }
        }
        
        return c;
    }

    // Get List of Common Valid Vectors for Legend
    static std::vector<SiBCSClassification> getCommonVectors(int level) {
        std::vector<SiBCSClassification> list;
        
        // Helper to add
        auto add = [&](SiBCSOrder o, SiBCSSuborder s = SiBCSSuborder::None, SiBCSGreatGroup g = SiBCSGreatGroup::None) {
            SiBCSClassification c; c.order = o; c.suborder = s; c.greatGroup = g;
            list.push_back(c);
        };

        if (level == 1) {
            add(SiBCSOrder::Latossolo); 
            add(SiBCSOrder::Argissolo); 
            add(SiBCSOrder::Neossolo);
            add(SiBCSOrder::Cambissolo);
            add(SiBCSOrder::Gleissolo);
        }
        else if (level == 2) {
            add(SiBCSOrder::Latossolo, SiBCSSuborder::Vermelho);
            add(SiBCSOrder::Latossolo, SiBCSSuborder::Amarelo);
            add(SiBCSOrder::Latossolo, SiBCSSuborder::VermelhoAmarelo);
            add(SiBCSOrder::Argissolo, SiBCSSuborder::Vermelho);
            add(SiBCSOrder::Argissolo, SiBCSSuborder::Amarelo);
            add(SiBCSOrder::Neossolo, SiBCSSuborder::Litoc);
            add(SiBCSOrder::Gleissolo, SiBCSSuborder::Haplic);
        }
        else if (level == 3) {
             // Expands combinatorialy, so list common ones
            add(SiBCSOrder::Latossolo, SiBCSSuborder::Vermelho, SiBCSGreatGroup::Distrofico);
            add(SiBCSOrder::Latossolo, SiBCSSuborder::Vermelho, SiBCSGreatGroup::Eutrofico);
            add(SiBCSOrder::Latossolo, SiBCSSuborder::Amarelo, SiBCSGreatGroup::Distrofico);
            add(SiBCSOrder::Argissolo, SiBCSSuborder::Vermelho, SiBCSGreatGroup::Distrofico);
            add(SiBCSOrder::Argissolo, SiBCSSuborder::Vermelho, SiBCSGreatGroup::Eutrofico);
            add(SiBCSOrder::Neossolo, SiBCSSuborder::Litoc, SiBCSGreatGroup::Distrofico); // Usually Eutrofico actually?
             add(SiBCSOrder::Neossolo, SiBCSSuborder::Litoc, SiBCSGreatGroup::Eutrofico);
        }

        return list;
    }
};

} // namespace Core::Domain::Soils
