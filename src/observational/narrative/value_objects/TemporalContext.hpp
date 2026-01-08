#pragma once

#include <string>
#include <nlohmann/json.hpp>

namespace SisterSTRATA::Observational::Narrative {

/**
 * @brief Value Object representing declared time in a narrative.
 */
class TemporalContext {
public:
    enum class RelativeTiming {
        ANCESTRAL,      // Deep past
        PAST,           // General past
        RECENT_PAST,    // Near past
        CONTEMPORARY,   // Present
        FUTURE_VISION,  // Projected
        TIMELESS,       // General truth
        INDETERMINATE   // Unclear
    };

    TemporalContext() = default;

    TemporalContext(RelativeTiming category, std::string label)
        : m_category(category), m_label(std::move(label)) {}

    RelativeTiming getCategory() const { return m_category; }
    const std::string& getLabel() const { return m_label; }

    bool operator==(const TemporalContext& other) const {
        return m_category == other.m_category &&
               m_label == other.m_label;
    }

    friend void to_json(nlohmann::json& j, const TemporalContext& obj) {
        j = nlohmann::json{
            {"category", static_cast<int>(obj.m_category)},
            {"label", obj.m_label}
        };
    }

    friend void from_json(const nlohmann::json& j, TemporalContext& obj) {
        obj.m_category = static_cast<RelativeTiming>(j.at("category").get<int>());
        obj.m_label = j.at("label").get<std::string>();
    }

private:
    RelativeTiming m_category = RelativeTiming::CONTEMPORARY;
    std::string m_label;
};

} // namespace SisterSTRATA::Observational::Narrative
