#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include "src/observational/narrative/value_objects/SourceReference.hpp"
#include "src/observational/narrative/value_objects/SemanticAxis.hpp"
#include "src/observational/narrative/value_objects/TemporalContext.hpp"

namespace SisterSTRATA::Observational::Narrative {

using NarrativeStateID = std::string;

/**
 * @brief Represents a semantic state declared from a narrative source.
 * 
 * This Entity is strictly observational. It does not represent a physical state
 * of the simulation but an interpretative configuration derived from a source.
 * 
 * INVARIANT: Immutable after creation.
 */
class NarrativeState {
public:
    NarrativeState(
        NarrativeStateID id,
        SourceReference source,
        TemporalContext temporalContext,
        std::vector<SemanticAxis> axes,
        std::map<std::string, std::string> metadata
    ) : m_id(std::move(id)),
        m_source(std::move(source)),
        m_temporalContext(std::move(temporalContext)),
        m_axes(std::move(axes)),
        m_metadata(std::move(metadata)) {}

    const NarrativeStateID& getId() const { return m_id; }
    const SourceReference& getSource() const { return m_source; }
    const TemporalContext& getTemporalContext() const { return m_temporalContext; }
    const std::vector<SemanticAxis>& getAxes() const { return m_axes; }
    
    // Interpretation metadata (e.g., origin of interpretation, confidence, analyst notes)
    const std::map<std::string, std::string>& getMetadata() const { return m_metadata; }

    // Domain Logic: None. By definition, this is an observation.

private:
    NarrativeStateID m_id;
    SourceReference m_source;
    TemporalContext m_temporalContext;
    std::vector<SemanticAxis> m_axes;
    std::map<std::string, std::string> m_metadata;
};

} // namespace SisterSTRATA::Observational::Narrative
