#pragma once

#include "application/dtos/MembraneEvidenceDTOs.hpp"
#include "application/ports/MembranePorts.hpp"

#include <stdexcept>
#include <string>

namespace Application::Services::MembraneContract {

class ContractViolation : public std::logic_error {
public:
    explicit ContractViolation(const std::string& message)
        : std::logic_error(message) {}
};

inline void validateEnvelope(const DTO::MembraneEnvelopeDTO& envelope) {
    if (envelope.type() != "observational_evidence") {
        throw ContractViolation("invalid membrane envelope type");
    }
    if (envelope.causalInterpretationAllowed()) {
        throw ContractViolation("causal interpretation is forbidden across membranes");
    }
    if (envelope.decisionDirective().has_value()) {
        throw ContractViolation("decision directives are forbidden across membranes");
    }
    const auto& source = envelope.sourceReference();
    if (source.context.empty() || source.artifact.empty() || source.timestamp.empty()) {
        throw ContractViolation("sourceReference is incomplete");
    }
}

class ObservabilityToInfrastructureAdapter final : public Ports::IObservabilityToInfrastructurePort {
public:
    void ingest(
        const DTO::SoilElectricalObservabilityDTO& observability,
        const DTO::HardwareFeasibilityDTO& feasibility,
        const DTO::MembraneEnvelopeDTO& envelope) override {
        (void)observability;
        (void)feasibility;
        validateEnvelope(envelope);
        ++acceptedCount_;
    }

    int acceptedCount() const { return acceptedCount_; }

private:
    int acceptedCount_{0};
};

class InfrastructureToFourthDimensionAdapter final : public Ports::IInfrastructureToFourthDimensionPort {
public:
    void ingest(
        const DTO::InfrastructureResilienceEvidenceDTO& evidence,
        const DTO::MembraneEnvelopeDTO& envelope) override {
        (void)evidence;
        validateEnvelope(envelope);
        ++acceptedCount_;
    }

    int acceptedCount() const { return acceptedCount_; }

private:
    int acceptedCount_{0};
};

class ObservabilityToFourthDimensionAdapter final : public Ports::IObservabilityToFourthDimensionPort {
public:
    void ingest(
        const DTO::ObservabilityVisibilityEvidenceDTO& evidence,
        const DTO::MembraneEnvelopeDTO& envelope) override {
        (void)evidence;
        validateEnvelope(envelope);
        ++acceptedCount_;
    }

    int acceptedCount() const { return acceptedCount_; }

private:
    int acceptedCount_{0};
};

} // namespace Application::Services::MembraneContract
