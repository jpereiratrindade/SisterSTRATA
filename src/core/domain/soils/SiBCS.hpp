#pragma once
#include <string>
#include <vector>
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

// Filter Struct for Multi-Select
struct SiBCSFilter {
    std::vector<SiBCSOrder> allowedOrders;
    std::vector<SiBCSSuborder> allowedSuborders;
    std::vector<SiBCSGreatGroup> allowedGreatGroups;
    // ... extend if needed for lower levels, keeping it simple for now
    
    bool isEmpty() const {
        return allowedOrders.empty() && allowedSuborders.empty() && allowedGreatGroups.empty();
    }
};

// Visualization Helper
struct SiBCSHelper {
    // ... (existing methods) ...

    static bool matches(const SiBCSClassification& soil, const SiBCSFilter& filter) {
        if (!filter.allowedOrders.empty()) {
            bool found = false;
            for (auto o : filter.allowedOrders) if (o == soil.order) found = true;
            if (!found) return false;
        }
        if (!filter.allowedSuborders.empty()) {
            bool found = false;
            for (auto s : filter.allowedSuborders) if (s == soil.suborder) found = true;
            if (!found) return false;
        }
        if (!filter.allowedGreatGroups.empty()) {
            bool found = false;
            for (auto g : filter.allowedGreatGroups) if (g == soil.greatGroup) found = true;
             if (!found) return false;
        }
        return true;
    }

    // ... (existing methods like getCommonVectors) ...

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
        // 1. Start with Base Order Color
        glm::vec3 c(0.5f);
        switch(soil.order) {
            case SiBCSOrder::Latossolo: c = glm::vec3(0.75f, 0.15f, 0.15f); break; // Deep Red
            case SiBCSOrder::Argissolo: c = glm::vec3(0.9f, 0.5f, 0.2f); break; // Orange
            case SiBCSOrder::Neossolo: c = glm::vec3(0.9f, 0.9f, 0.7f); break; // Pale
            case SiBCSOrder::Cambissolo: c = glm::vec3(0.6f, 0.4f, 0.2f); break; // Brown
            case SiBCSOrder::Gleissolo: c = glm::vec3(0.4f, 0.6f, 0.7f); break; // Blue-Grey
            default: c = glm::vec3(0.5f); break;
        }

        if (level == 1) return c;

        // 2. Refine for Suborder (if applicable)
        if (level >= 2) {
            if (soil.order == SiBCSOrder::Latossolo) {
                switch(soil.suborder) {
                    case SiBCSSuborder::Vermelho: c = glm::vec3(0.6f, 0.1f, 0.1f); break;
                    case SiBCSSuborder::Amarelo: c = glm::vec3(0.9f, 0.8f, 0.1f); break;
                    case SiBCSSuborder::VermelhoAmarelo: c = glm::vec3(0.9f, 0.4f, 0.1f); break;
                    default: break; // Keep base
                }
            }
            else if (soil.order == SiBCSOrder::Argissolo) {
                switch(soil.suborder) {
                    case SiBCSSuborder::Vermelho: c = glm::vec3(0.7f, 0.2f, 0.1f); break;
                    case SiBCSSuborder::Amarelo: c = glm::vec3(0.95f, 0.7f, 0.2f); break;
                    default: break;
                }
            }
            else if (soil.order == SiBCSOrder::Gleissolo) {
                 c = glm::vec3(0.3f, 0.5f, 0.6f); 
            }
            else if (soil.order == SiBCSOrder::Neossolo) {
                 if (soil.suborder == SiBCSSuborder::Litoc) c = glm::vec3(0.6f, 0.6f, 0.6f); 
            }
        }

        // 3. Refine for Great Group (Fertility)
        if (level >= 3) {
            if (soil.greatGroup == SiBCSGreatGroup::Eutrofico) {
                c = glm::clamp(c * 1.2f, 0.0f, 1.0f); // Brighter
            } else if (soil.greatGroup == SiBCSGreatGroup::Distrofico) {
                c *= 0.8f; // Darker
            }
        }
        
        return c;
    }

    // List Helpers for UI
    static std::vector<SiBCSOrder> getAllOrders() {
        return { 
            SiBCSOrder::Latossolo, SiBCSOrder::Argissolo, SiBCSOrder::Neossolo, 
            SiBCSOrder::Cambissolo, SiBCSOrder::Gleissolo, SiBCSOrder::Planossolo,
            SiBCSOrder::Espodossolo, SiBCSOrder::Vertissolo, SiBCSOrder::Nitossolo,
            SiBCSOrder::Chernossolo, SiBCSOrder::Luvissolo, SiBCSOrder::Plintossolo,
            SiBCSOrder::Organossolo 
        };
    }

    static std::vector<SiBCSSuborder> getAllSuborders() {
        return { 
            SiBCSSuborder::Vermelho, SiBCSSuborder::Amarelo, SiBCSSuborder::VermelhoAmarelo,
            SiBCSSuborder::Bruno, SiBCSSuborder::Haplic, SiBCSSuborder::Litoc, 
            SiBCSSuborder::Gleico 
        };
    }

    static std::vector<SiBCSGreatGroup> getAllGreatGroups() {
        return { 
            SiBCSGreatGroup::Distrofico, SiBCSGreatGroup::Eutrofico, 
            SiBCSGreatGroup::Alico, SiBCSGreatGroup::Humico 
        };
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

        else if (level >= 4) { // Levels 4, 5, 6 (show typical simplified examples)
            // Just show key examples to avoid crowding
            add(SiBCSOrder::Latossolo, SiBCSSuborder::Vermelho, SiBCSGreatGroup::Distrofico); 
            add(SiBCSOrder::Latossolo, SiBCSSuborder::Amarelo, SiBCSGreatGroup::Distrofico);
            add(SiBCSOrder::Argissolo, SiBCSSuborder::Vermelho, SiBCSGreatGroup::Eutrofico);
            add(SiBCSOrder::Neossolo, SiBCSSuborder::Litoc, SiBCSGreatGroup::Eutrofico);
        }

        return list;
    }
};

} // namespace Core::Domain::Soils
