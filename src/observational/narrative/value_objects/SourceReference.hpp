#pragma once

#include <string>
#include <optional>

namespace SisterSTRATA::Observational::Narrative {

/**
 * @brief Value Object representing the origin of a narrative observation.
 * 
 * Provides strict traceability to the source material (interview, document, etc.).
 * Immutable by design.
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

private:
    SourceType m_type;
    std::string m_sourceId;
    std::string m_productionDate; // string to avoid complexity, or could be ISO date
    std::optional<std::string> m_author;
};

} // namespace SisterSTRATA::Observational::Narrative
