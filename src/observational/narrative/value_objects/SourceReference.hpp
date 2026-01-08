#pragma once

#include <string>
#include <optional>
#include <nlohmann/json.hpp>

namespace SisterSTRATA::Observational::Narrative {

/**
 * @brief Value Object representing the origin of a narrative observation.
 */
class SourceReference {
public:
    enum class SourceType {
        INTERVIEW,
        TECHNICAL_REPORT,
        HISTORICAL_RECORD,
        SCIENTIFIC_ARTICLE,
        INSTITUTIONAL_DOCUMENT,
        MEDIA_ARTICLE,
        FIELD_NOTE,
        OTHER
    };

    SourceReference() = default; // needed for JSON

    SourceReference(SourceType type, std::string sourceId, std::string productionDate, std::optional<std::string> author = std::nullopt)
        : m_type(type), m_sourceId(std::move(sourceId)), m_productionDate(std::move(productionDate)), m_author(std::move(author)) {}

    SourceType getType() const { return m_type; }
    const std::string& getSourceId() const { return m_sourceId; }
    const std::string& getProductionDate() const { return m_productionDate; }
    std::optional<std::string> getAuthor() const { return m_author; }

    bool operator==(const SourceReference& other) const {
        return m_type == other.m_type &&
               m_sourceId == other.m_sourceId &&
               m_productionDate == other.m_productionDate &&
               m_author == other.m_author;
    }

    // Friend serialization functions
    friend void to_json(nlohmann::json& j, const SourceReference& obj) {
        j = nlohmann::json{
            {"type", static_cast<int>(obj.m_type)},
            {"sourceId", obj.m_sourceId},
            {"productionDate", obj.m_productionDate}
        };
        if (obj.m_author.has_value()) {
            j["author"] = obj.m_author.value();
        }
    }

    friend void from_json(const nlohmann::json& j, SourceReference& obj) {
        obj.m_type = static_cast<SourceType>(j.at("type").get<int>());
        obj.m_sourceId = j.at("sourceId").get<std::string>();
        obj.m_productionDate = j.at("productionDate").get<std::string>();
        if (j.contains("author")) {
            obj.m_author = j.at("author").get<std::string>();
        } else {
            obj.m_author = std::nullopt;
        }
    }

private:
    SourceType m_type = SourceType::OTHER;
    std::string m_sourceId;
    std::string m_productionDate;
    std::optional<std::string> m_author;
};

} // namespace SisterSTRATA::Observational::Narrative
