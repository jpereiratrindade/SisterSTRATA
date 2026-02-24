#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include "observational/narrative/value_objects/SourceReference.hpp"
#include "observational/narrative/value_objects/SemanticAxis.hpp"
#include "observational/narrative/value_objects/TemporalContext.hpp"
#include "observational/narrative/value_objects/ObservationIntent.hpp"
#include "observational/narrative/value_objects/SpatialScope.hpp"

namespace SisterSTRATA::Observational::Narrative {

using NarrativeStateID = std::string;

/**
 * @brief Represents a semantic state declared from a narrative source.
 */
class NarrativeState {
public:
    NarrativeState() = default; // Needed for JSON

    NarrativeState(
        NarrativeStateID id,
        SourceReference source,
        TemporalContext temporalContext,
        ObservationIntent intent,
        std::vector<SemanticAxis> axes,
        std::map<std::string, std::string> metadata,
        std::optional<SpatialScope> spatialScope = std::nullopt
    ) : m_id(std::move(id)),
        m_source(std::move(source)),
        m_temporalContext(std::move(temporalContext)),
        m_intent(intent),
        m_axes(std::move(axes)),
        m_metadata(std::move(metadata)),
        m_spatialScope(std::move(spatialScope)) {}

    const NarrativeStateID& getId() const { return m_id; }
    const SourceReference& getSource() const { return m_source; }
    const TemporalContext& getTemporalContext() const { return m_temporalContext; }
    ObservationIntent getIntent() const { return m_intent; }
    const std::vector<SemanticAxis>& getAxes() const { return m_axes; }
    const std::map<std::string, std::string>& getMetadata() const { return m_metadata; }
    const std::optional<SpatialScope>& getSpatialScope() const { return m_spatialScope; }

    friend void to_json(nlohmann::json& j, const NarrativeState& obj) {
        j = nlohmann::json{
            {"id", obj.m_id},
            {"source", obj.m_source},
            {"temporalContext", obj.m_temporalContext},
            {"intent", obj.m_intent},
            {"axes", obj.m_axes},
            {"metadata", obj.m_metadata}
        };
        if (obj.m_spatialScope.has_value()) {
            j["spatialScope"] = obj.m_spatialScope.value();
        }
    }

    friend void from_json(const nlohmann::json& j, NarrativeState& obj) {
        obj.m_id = j.at("id").get<NarrativeStateID>();
        obj.m_source = j.at("source").get<SourceReference>();
        obj.m_temporalContext = j.at("temporalContext").get<TemporalContext>();
        obj.m_intent = j.at("intent").get<ObservationIntent>();
        obj.m_axes = j.at("axes").get<std::vector<SemanticAxis>>();
        obj.m_metadata = j.at("metadata").get<std::map<std::string, std::string>>();
        
        if (j.contains("spatialScope")) {
            obj.m_spatialScope = j.at("spatialScope").get<SpatialScope>();
        } else {
            obj.m_spatialScope = std::nullopt;
        }
    }

private:
    NarrativeStateID m_id;
    SourceReference m_source;
    TemporalContext m_temporalContext;
    ObservationIntent m_intent = ObservationIntent(ObservationIntent::IntentType::DESCRIPTIVE_RECORD);
    std::vector<SemanticAxis> m_axes;
    std::map<std::string, std::string> m_metadata;
    std::optional<SpatialScope> m_spatialScope;
};

} // namespace SisterSTRATA::Observational::Narrative
