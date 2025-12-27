#pragma once
#include "SiBCS.hpp"
#include "Scorpan.hpp"
#include <cmath>

namespace Core::Domain::Soils {

class SoilPredictor {
public:
    static SiBCSClass predict(const ScorpanParams& global, float slopeDeg, float elevation, float relElevation) {
        // SCORPAN Heuristics
        
        // 1. Check Relief/Slope (Strongest factor for Neossolo)
        // Steep slope (> 30 deg) usually prevents deep soil formation -> Neossolo Litólico
        if (slopeDeg > 35.0f) {
            return SiBCSClass::Neossolo;
        }

        // 2. Hydromorphic check (Low relative elevation + High Rainfall = Gleissolo)
        // relElevation < 0.1 (Depression/Valley bottom)
        bool potentiallyHydromorphic = (relElevation < 0.05f); // Bottom 5% of terrain
        if (potentiallyHydromorphic && global.rainfall > 1200.0f) {
            return SiBCSClass::Gleissolo; 
        }

        // 3. Age & Weathering check
        // Old soils (Latossolo) require flatness and time
        if (global.ageFactor > 0.7f && slopeDeg < 8.0f) {
            return SiBCSClass::Latossolo;
        }

        // 4. Transitional
        if (slopeDeg > 15.0f && slopeDeg <= 35.0f) {
            return SiBCSClass::Cambissolo; // Shallow but developing
        }
        
        // 5. Argissolo (Bt horizon)
        // Needs some seasonality and age, but not as weathered as Latossolo
        if (global.ageFactor > 0.3f && slopeDeg < 20.0f) {
            return SiBCSClass::Argissolo;
        }

        // Default to Cambissolo if nothing else fits well
        return SiBCSClass::Cambissolo;
    }
};

} // namespace Core::Domain::Soils
