#pragma once

#include <string>
#include <nlohmann/json.hpp>

namespace SisterSTRATA::Observational::Narrative {

/**
 * @brief Value Object representing a thematic axis or discursive dimension.
 */
class SemanticAxis {
public:
    enum class AbstractionLevel {
        LOCAL,          // Specific to a patch or small area
        REGIONAL,       // Covers a larger region/landscape
        INSTITUTIONAL,  // Related to policy/governance
        GLOBAL          // Broad context
    };

    SemanticAxis() = default;

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

    friend void to_json(nlohmann::json& j, const SemanticAxis& obj) {
        j = nlohmann::json{
            {"label", obj.m_label},
            {"description", obj.m_description},
            {"level", static_cast<int>(obj.m_level)}
        };
    }

    friend void from_json(const nlohmann::json& j, SemanticAxis& obj) {
        obj.m_label = j.at("label").get<std::string>();
        obj.m_description = j.at("description").get<std::string>();
        obj.m_level = static_cast<AbstractionLevel>(j.at("level").get<int>());
    }

private:
    std::string m_label;
    std::string m_description;
    AbstractionLevel m_level = AbstractionLevel::LOCAL;
};

} // namespace SisterSTRATA::Observational::Narrative
