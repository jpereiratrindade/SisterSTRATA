#pragma once

#include <string>

namespace SisterSTRATA::Observational::Narrative {

/**
 * @brief Value Object representing declared time in a narrative.
 * 
 * IMPORTANT: This represents time as declared in the source, NOT simulated physical time.
 * It uses declarative relative ordering rather than precise timestamps to avoid
 * conflation with the core simulation clock.
 */
class TemporalContext {
public:
    enum class RelativeTiming {
        ANCESTRAL,      // Deep past, specific to narrative context
        PAST,           // General past
        RECENT_PAST,    // Near past
        CONTEMPORARY,   // Present at the time of observation
        FUTURE_VISION,  // Projected or desired future
        TIMELESS        // General truth or statement without specific time
    };

    TemporalContext(RelativeTiming timing, std::string declaredTimeLabel)
        : m_timing(timing), m_declaredTimeLabel(std::move(declaredTimeLabel)) {}

    RelativeTiming getTiming() const { return m_timing; }
    const std::string& getDeclaredTimeLabel() const { return m_declaredTimeLabel; }

    bool operator==(const TemporalContext& other) const {
        return m_timing == other.m_timing &&
               m_declaredTimeLabel == other.m_declaredTimeLabel;
    }

private:
    RelativeTiming m_timing;
    std::string m_declaredTimeLabel; // e.g., "Before the dam", "During the 1990s"
};

} // namespace SisterSTRATA::Observational::Narrative
