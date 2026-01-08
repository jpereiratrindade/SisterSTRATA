#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include "src/observational/recommendation/value_objects/SourceReference.hpp"
#include "src/observational/recommendation/value_objects/TemporalContext.hpp"

namespace SisterSTRATA::Observational::Recommendation {

using RecommendationSnapshotID = std::string;

/**
 * @brief Represents a single recommendation observation.
 */
class RecommendationSnapshot {
public:
    RecommendationSnapshot() = default; // Needed for JSON

    RecommendationSnapshot(
        RecommendationSnapshotID id,
        std::string recommendationText,
        std::vector<std::string> contextConditions,
        std::string intendedAction,
        std::string expectedOutcome,
        SourceReference sourceReference,
        TemporalContext temporalContext
    ) : m_id(std::move(id)),
        m_recommendationText(std::move(recommendationText)),
        m_contextConditions(std::move(contextConditions)),
        m_intendedAction(std::move(intendedAction)),
        m_expectedOutcome(std::move(expectedOutcome)),
        m_sourceReference(std::move(sourceReference)),
        m_temporalContext(std::move(temporalContext)) {}

    const RecommendationSnapshotID& getId() const { return m_id; }
    const std::string& getRecommendationText() const { return m_recommendationText; }
    const std::vector<std::string>& getContextConditions() const { return m_contextConditions; }
    const std::string& getIntendedAction() const { return m_intendedAction; }
    const std::string& getExpectedOutcome() const { return m_expectedOutcome; }
    const SourceReference& getSourceReference() const { return m_sourceReference; }
    const TemporalContext& getTemporalContext() const { return m_temporalContext; }

    friend void to_json(nlohmann::json& j, const RecommendationSnapshot& obj) {
        j = nlohmann::json{
            {"id", obj.m_id},
            {"recommendationText", obj.m_recommendationText},
            {"contextConditions", obj.m_contextConditions},
            {"intendedAction", obj.m_intendedAction},
            {"expectedOutcome", obj.m_expectedOutcome},
            {"sourceReference", obj.m_sourceReference},
            {"temporalContext", obj.m_temporalContext}
        };
    }

    friend void from_json(const nlohmann::json& j, RecommendationSnapshot& obj) {
        obj.m_id = j.at("id").get<RecommendationSnapshotID>();
        obj.m_recommendationText = j.at("recommendationText").get<std::string>();
        obj.m_contextConditions = j.at("contextConditions").get<std::vector<std::string>>();
        obj.m_intendedAction = j.at("intendedAction").get<std::string>();
        obj.m_expectedOutcome = j.at("expectedOutcome").get<std::string>();
        obj.m_sourceReference = j.at("sourceReference").get<SourceReference>();
        obj.m_temporalContext = j.at("temporalContext").get<TemporalContext>();
    }

private:
    RecommendationSnapshotID m_id;
    std::string m_recommendationText;
    std::vector<std::string> m_contextConditions;
    std::string m_intendedAction;
    std::string m_expectedOutcome;
    SourceReference m_sourceReference;
    TemporalContext m_temporalContext;
};

} // namespace SisterSTRATA::Observational::Recommendation
