#pragma once

#include <optional>
#include <string>

namespace Application::DTO {

struct EvidenceSourceReferenceDTO {
    std::string context;
    std::string artifact;
    std::string timestamp;
};

class MembraneEnvelopeDTO {
public:
    MembraneEnvelopeDTO(
        std::string type,
        bool causalInterpretationAllowed,
        std::optional<std::string> decisionDirective,
        EvidenceSourceReferenceDTO sourceReference)
        : type_(std::move(type)),
          causalInterpretationAllowed_(causalInterpretationAllowed),
          decisionDirective_(std::move(decisionDirective)),
          sourceReference_(std::move(sourceReference)) {}

    const std::string& type() const { return type_; }
    bool causalInterpretationAllowed() const { return causalInterpretationAllowed_; }
    const std::optional<std::string>& decisionDirective() const { return decisionDirective_; }
    const EvidenceSourceReferenceDTO& sourceReference() const { return sourceReference_; }

private:
    std::string type_;
    bool causalInterpretationAllowed_{false};
    std::optional<std::string> decisionDirective_{std::nullopt};
    EvidenceSourceReferenceDTO sourceReference_{};
};

class SoilElectricalObservabilityDTO {
public:
    SoilElectricalObservabilityDTO(
        std::string transitionType,
        double deltaResistivity,
        double snrRequired,
        std::string recommendedFrequencyBand)
        : transitionType_(std::move(transitionType)),
          deltaResistivity_(deltaResistivity),
          snrRequired_(snrRequired),
          recommendedFrequencyBand_(std::move(recommendedFrequencyBand)) {}

    const std::string& transitionType() const { return transitionType_; }
    double deltaResistivity() const { return deltaResistivity_; }
    double snrRequired() const { return snrRequired_; }
    const std::string& recommendedFrequencyBand() const { return recommendedFrequencyBand_; }

private:
    std::string transitionType_;
    double deltaResistivity_{0.0};
    double snrRequired_{0.0};
    std::string recommendedFrequencyBand_;
};

class HardwareFeasibilityDTO {
public:
    HardwareFeasibilityDTO(
        int adcResolutionBits,
        double achievableSnr,
        double minimumDetectableDelta,
        bool meetsRequirement,
        std::string limitingFactor)
        : adcResolutionBits_(adcResolutionBits),
          achievableSnr_(achievableSnr),
          minimumDetectableDelta_(minimumDetectableDelta),
          meetsRequirement_(meetsRequirement),
          limitingFactor_(std::move(limitingFactor)) {}

    int adcResolutionBits() const { return adcResolutionBits_; }
    double achievableSnr() const { return achievableSnr_; }
    double minimumDetectableDelta() const { return minimumDetectableDelta_; }
    bool meetsRequirement() const { return meetsRequirement_; }
    const std::string& limitingFactor() const { return limitingFactor_; }

private:
    int adcResolutionBits_{0};
    double achievableSnr_{0.0};
    double minimumDetectableDelta_{0.0};
    bool meetsRequirement_{false};
    std::string limitingFactor_;
};

class InfrastructureResilienceEvidenceDTO {
public:
    InfrastructureResilienceEvidenceDTO(
        std::string ecologicalScenario,
        double poolStorageWh,
        double identityReliabilityIndex,
        double identityRequestedWh,
        double identityAllocatedWh,
        double identityConsumedWh,
        double soilReliabilityIndex,
        double soilRequestedWh,
        double soilAllocatedWh,
        double soilConsumedWh)
        : ecologicalScenario_(std::move(ecologicalScenario)),
          poolStorageWh_(poolStorageWh),
          identityReliabilityIndex_(identityReliabilityIndex),
          identityRequestedWh_(identityRequestedWh),
          identityAllocatedWh_(identityAllocatedWh),
          identityConsumedWh_(identityConsumedWh),
          soilReliabilityIndex_(soilReliabilityIndex),
          soilRequestedWh_(soilRequestedWh),
          soilAllocatedWh_(soilAllocatedWh),
          soilConsumedWh_(soilConsumedWh) {}

    const std::string& ecologicalScenario() const { return ecologicalScenario_; }
    double poolStorageWh() const { return poolStorageWh_; }
    double identityReliabilityIndex() const { return identityReliabilityIndex_; }
    double identityRequestedWh() const { return identityRequestedWh_; }
    double identityAllocatedWh() const { return identityAllocatedWh_; }
    double identityConsumedWh() const { return identityConsumedWh_; }
    double soilReliabilityIndex() const { return soilReliabilityIndex_; }
    double soilRequestedWh() const { return soilRequestedWh_; }
    double soilAllocatedWh() const { return soilAllocatedWh_; }
    double soilConsumedWh() const { return soilConsumedWh_; }

private:
    std::string ecologicalScenario_;
    double poolStorageWh_{0.0};
    double identityReliabilityIndex_{0.0};
    double identityRequestedWh_{0.0};
    double identityAllocatedWh_{0.0};
    double identityConsumedWh_{0.0};
    double soilReliabilityIndex_{0.0};
    double soilRequestedWh_{0.0};
    double soilAllocatedWh_{0.0};
    double soilConsumedWh_{0.0};
};

class ObservabilityVisibilityEvidenceDTO {
public:
    ObservabilityVisibilityEvidenceDTO(
        std::string visibilityClass,
        double uncertaintyIndex)
        : visibilityClass_(std::move(visibilityClass)),
          uncertaintyIndex_(uncertaintyIndex) {}

    const std::string& visibilityClass() const { return visibilityClass_; }
    double uncertaintyIndex() const { return uncertaintyIndex_; }

private:
    std::string visibilityClass_;
    double uncertaintyIndex_{0.0};
};

} // namespace Application::DTO
