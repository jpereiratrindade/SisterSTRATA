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
struct SiBCSClassification {
    SiBCSOrder order = SiBCSOrder::Unknown;
    SiBCSSuborder suborder = SiBCSSuborder::None;
    SiBCSGreatGroup greatGroup = SiBCSGreatGroup::None;
    SiBCSSubgroup subgroup = SiBCSSubgroup::None;
    SiBCSFamily family = SiBCSFamily::None;
    SiBCSSeries series = SiBCSSeries::None;
};

// Visualization Helper
struct SiBCSHelper {
    static std::string getName(SiBCSOrder type) {
        switch(type) {
            case SiBCSOrder::Latossolo: return "Latossolo (L)";
            case SiBCSOrder::Argissolo: return "Argissolo (P)";
            case SiBCSOrder::Neossolo: return "Neossolo (R)";
            case SiBCSOrder::Cambissolo: return "Cambissolo (C)";
            case SiBCSOrder::Gleissolo: return "Gleissolo (G)";
            case SiBCSOrder::Planossolo: return "Planossolo (S)";
            case SiBCSOrder::Espodossolo: return "Espodossolo (E)";
            case SiBCSOrder::Vertissolo: return "Vertissolo (V)";
            case SiBCSOrder::Nitossolo: return "Nitossolo (N)";
            case SiBCSOrder::Organossolo: return "Organossolo (O)";
            default: return "Outro";
        }
    }
    
    static std::string getName(SiBCSSuborder type) {
        switch(type) {
            case SiBCSSuborder::Vermelho: return "Vermelho";
            case SiBCSSuborder::Amarelo: return "Amarelo";
            case SiBCSSuborder::Bruno: return "Bruno";
            case SiBCSSuborder::Litoc: return "Litólico";
            case SiBCSSuborder::Gleico: return "Gleico";
            case SiBCSSuborder::Haplic: return "Háplico";
            default: return "";
        }
    }

     static std::string getName(SiBCSGreatGroup type) {
        switch(type) {
            case SiBCSGreatGroup::Distrofico: return "Distrófico";
            case SiBCSGreatGroup::Eutrofico: return "Eutrófico";
             case SiBCSGreatGroup::Alico: return "Álico";
            default: return "";
        }
    }

    // Color Lookup based on Visualization Level
    static glm::vec3 getColor(const SiBCSClassification& soil, int level) {
        // Level 1: Order Colors
        if (level == 1) {
            switch(soil.order) {
                case SiBCSOrder::Latossolo: return glm::vec3(0.75f, 0.15f, 0.15f); // Deep Red (Standard)
                case SiBCSOrder::Argissolo: return glm::vec3(0.9f, 0.5f, 0.2f); // Orange-Yellow
                case SiBCSOrder::Neossolo: return glm::vec3(0.9f, 0.9f, 0.7f);  // Pale/Beige (Young) or Grey for Litolic
                case SiBCSOrder::Cambissolo: return glm::vec3(0.6f, 0.4f, 0.2f); // Brown
                case SiBCSOrder::Gleissolo: return glm::vec3(0.4f, 0.6f, 0.7f); // Blue-Grey (Water)
                case SiBCSOrder::Planossolo: return glm::vec3(0.8f, 0.8f, 0.9f); // Very Pale/Whiteish
                case SiBCSOrder::Espodossolo: return glm::vec3(0.4f, 0.4f, 0.4f); // Grey (Podzol)
                case SiBCSOrder::Chernossolo: return glm::vec3(0.1f, 0.1f, 0.1f); // Black
                case SiBCSOrder::Nitossolo: return glm::vec3(0.6f, 0.1f, 0.4f); // Purplish Red
                default: return glm::vec3(1.0f);
            }
        }
        
        // Level 2: Suborder (e.g. Red vs Yellow)
        if (level == 2) {
            switch(soil.suborder) {
                case SiBCSSuborder::Vermelho: return glm::vec3(0.8f, 0.1f, 0.1f); 
                case SiBCSSuborder::Amarelo: return glm::vec3(0.9f, 0.9f, 0.2f); 
                case SiBCSSuborder::VermelhoAmarelo: return glm::vec3(0.9f, 0.5f, 0.1f); // Orange
                case SiBCSSuborder::Bruno: return glm::vec3(0.4f, 0.3f, 0.1f); 
                case SiBCSSuborder::Litoc: return glm::vec3(0.7f, 0.7f, 0.7f); 
                case SiBCSSuborder::Gleico: return glm::vec3(0.3f, 0.5f, 0.7f); 
                default: return glm::vec3(0.8f);
            }
        }

        // Level 3: Great Group (Fertility)
        if (level == 3) {
            switch(soil.greatGroup) {
                case SiBCSGreatGroup::Eutrofico: return glm::vec3(0.2f, 0.7f, 0.2f); // Green (Fertile)
                case SiBCSGreatGroup::Distrofico: return glm::vec3(0.7f, 0.7f, 0.2f); // Yellowish (Poor)
                case SiBCSGreatGroup::Alico: return glm::vec3(0.5f, 0.5f, 0.2f); // Toxic/Acid
                default: return glm::vec3(0.5f);
            }
        }

        // Default to Order
        return getColor(soil, 1);
    }
    static const std::vector<SiBCSOrder>& getAllOrders() {
        static const std::vector<SiBCSOrder> orders = {
            SiBCSOrder::Latossolo, SiBCSOrder::Argissolo, SiBCSOrder::Neossolo,
            SiBCSOrder::Cambissolo, SiBCSOrder::Gleissolo, SiBCSOrder::Planossolo,
            SiBCSOrder::Espodossolo, SiBCSOrder::Vertissolo, SiBCSOrder::Nitossolo,
            SiBCSOrder::Chernossolo, SiBCSOrder::Luvissolo, SiBCSOrder::Plintossolo,
            SiBCSOrder::Organossolo
        };
        return orders;
    }

    static const std::vector<SiBCSSuborder>& getAllSuborders() {
        static const std::vector<SiBCSSuborder> suborders = {
            SiBCSSuborder::Vermelho, SiBCSSuborder::Amarelo, SiBCSSuborder::VermelhoAmarelo,
            SiBCSSuborder::Bruno, SiBCSSuborder::Litoc, SiBCSSuborder::Gleico, 
            SiBCSSuborder::Haplic, SiBCSSuborder::Tiptico
        };
        return suborders;
    }

    static const std::vector<SiBCSGreatGroup>& getAllGreatGroups() {
        static const std::vector<SiBCSGreatGroup> groups = {
            SiBCSGreatGroup::Distrofico, SiBCSGreatGroup::Eutrofico, 
            SiBCSGreatGroup::Alico, SiBCSGreatGroup::Humico
        };
        return groups;
    }
};

} // namespace Core::Domain::Soils
