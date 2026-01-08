#pragma once

#include <string>

namespace SisterSTRATA::Observational::Narrative {

/**
 * @brief Value Object representing a thematic axis or discursive dimension.
 * 
 * Used to classify the content of a narrative observation.
 * Immutable by design.
 */
class SemanticAxis {
public:
    enum class AbstractionLevel {
        LOCAL,          // Specific to a patch or small area
        REGIONAL,       // Covers a larger region/landscape
        INSTITUTIONAL,  // Related to policy/governance
        GLOBAL          // Broad context
    };

    SemanticAxis(std::string label, std::string description, AbstractionLevel level)
        : m_label(std::move(label)), m_description(std::move(description)), m_level(level) {}

    const std::string& getLabel() const { return m_label; }
    const std::string& getDescription() const { return m_description; }
    AbstractionLevel getLevel() const { return m_level; }

    bool operator==(const SemanticAxis& other) const {
        return m_label == other.m_label &&
               m_description == other.m_description &&
               m_level == other.m_level;
    }

private:
    std::string m_label;
    std::string m_description;
    AbstractionLevel m_level;
};

} // namespace SisterSTRATA::Observational::Narrative
