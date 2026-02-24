#pragma once

#include <string>
#include <vector>
#include <map>
#include <nlohmann/json.hpp>
#include "observational/discursive/value_objects/DeclaredProblem.hpp"
#include "observational/discursive/value_objects/DeclaredAction.hpp"
#include "observational/discursive/value_objects/AllegedMechanism.hpp"
#include "observational/discursive/value_objects/ExpectedEffect.hpp"
#include "observational/discursive/value_objects/SourceReference.hpp"
#include "observational/discursive/value_objects/TemporalContext.hpp"

namespace SisterSTRATA::Observational::Discursive {

using DiscursiveSystemID = std::string;

/**
 * @brief Represents a declared discursive system derived from textual sources.
 */
class DiscursiveSystem {
public:
    DiscursiveSystem() = default; // Needed for JSON

    DiscursiveSystem(
        DiscursiveSystemID id,
        std::vector<DeclaredProblem> declaredProblems,
        std::vector<DeclaredAction> declaredActions,
        std::vector<AllegedMechanism> allegedMechanisms,
        std::vector<ExpectedEffect> expectedEffects,
        std::vector<SourceReference> sourceReferences,
        TemporalContext temporalContext,
        std::map<std::string, std::string> interpretationMetadata
    ) : m_id(std::move(id)),
        m_declaredProblems(std::move(declaredProblems)),
        m_declaredActions(std::move(declaredActions)),
        m_allegedMechanisms(std::move(allegedMechanisms)),
        m_expectedEffects(std::move(expectedEffects)),
        m_sourceReferences(std::move(sourceReferences)),
        m_temporalContext(std::move(temporalContext)),
        m_interpretationMetadata(std::move(interpretationMetadata)) {}

    const DiscursiveSystemID& getId() const { return m_id; }
    const std::vector<DeclaredProblem>& getDeclaredProblems() const { return m_declaredProblems; }
    const std::vector<DeclaredAction>& getDeclaredActions() const { return m_declaredActions; }
    const std::vector<AllegedMechanism>& getAllegedMechanisms() const { return m_allegedMechanisms; }
    const std::vector<ExpectedEffect>& getExpectedEffects() const { return m_expectedEffects; }
    const std::vector<SourceReference>& getSourceReferences() const { return m_sourceReferences; }
    const TemporalContext& getTemporalContext() const { return m_temporalContext; }
    const std::map<std::string, std::string>& getInterpretationMetadata() const { return m_interpretationMetadata; }

    friend void to_json(nlohmann::json& j, const DiscursiveSystem& obj) {
        j = nlohmann::json{
            {"id", obj.m_id},
            {"declaredProblems", obj.m_declaredProblems},
            {"declaredActions", obj.m_declaredActions},
            {"allegedMechanisms", obj.m_allegedMechanisms},
            {"expectedEffects", obj.m_expectedEffects},
            {"sourceReferences", obj.m_sourceReferences},
            {"temporalContext", obj.m_temporalContext},
            {"interpretationMetadata", obj.m_interpretationMetadata}
        };
    }

    friend void from_json(const nlohmann::json& j, DiscursiveSystem& obj) {
        obj.m_id = j.at("id").get<DiscursiveSystemID>();
        obj.m_declaredProblems = j.at("declaredProblems").get<std::vector<DeclaredProblem>>();
        obj.m_declaredActions = j.at("declaredActions").get<std::vector<DeclaredAction>>();
        obj.m_allegedMechanisms = j.at("allegedMechanisms").get<std::vector<AllegedMechanism>>();
        obj.m_expectedEffects = j.at("expectedEffects").get<std::vector<ExpectedEffect>>();
        obj.m_sourceReferences = j.at("sourceReferences").get<std::vector<SourceReference>>();
        obj.m_temporalContext = j.at("temporalContext").get<TemporalContext>();
        obj.m_interpretationMetadata = j.at("interpretationMetadata").get<std::map<std::string, std::string>>();
    }

private:
    DiscursiveSystemID m_id;
    std::vector<DeclaredProblem> m_declaredProblems;
    std::vector<DeclaredAction> m_declaredActions;
    std::vector<AllegedMechanism> m_allegedMechanisms;
    std::vector<ExpectedEffect> m_expectedEffects;
    std::vector<SourceReference> m_sourceReferences;
    TemporalContext m_temporalContext;
    std::map<std::string, std::string> m_interpretationMetadata;
};

} // namespace SisterSTRATA::Observational::Discursive
