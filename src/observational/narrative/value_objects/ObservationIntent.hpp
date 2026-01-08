#pragma once

#include <string>
#include <nlohmann/json.hpp>

namespace SisterSTRATA::Observational::Narrative {

/**
 * @brief Represents the intent behind registering a narrative observation.
 */
class ObservationIntent {
public:
    enum class IntentType {
        DESCRIPTIVE_RECORD,      // Just noting what was said/written (neutral).
        EXPLORATORY_HYPOTHESIS,  // A "what if" or speculative interpretation grounded in text.
        CONTEXTUALIZATION,       // External context needed to understand the territory.
        METHODOLOGICAL_NOTE      // Meta-commentary on the source or process.
    };

    ObservationIntent() = default; // Needed for JSON

    explicit ObservationIntent(IntentType type) : m_type(type) {}

    IntentType getType() const { return m_type; }

    std::string toString() const {
        switch (m_type) {
            case IntentType::DESCRIPTIVE_RECORD: return "Descriptive Record";
            case IntentType::EXPLORATORY_HYPOTHESIS: return "Exploratory Hypothesis";
            case IntentType::CONTEXTUALIZATION: return "Contextualization";
            case IntentType::METHODOLOGICAL_NOTE: return "Methodological Note";
            default: return "Unknown";
        }
    }

    bool operator==(const ObservationIntent& other) const {
        return m_type == other.m_type;
    }

    friend void to_json(nlohmann::json& j, const ObservationIntent& obj) {
        j = nlohmann::json{{"type", static_cast<int>(obj.m_type)}};
    }

    friend void from_json(const nlohmann::json& j, ObservationIntent& obj) {
        obj.m_type = static_cast<IntentType>(j.at("type").get<int>());
    }

private:
    IntentType m_type = IntentType::DESCRIPTIVE_RECORD;
};

} // namespace SisterSTRATA::Observational::Narrative
