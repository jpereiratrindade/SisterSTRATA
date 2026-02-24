#pragma once

#include "application/dtos/MembraneEvidenceDTOs.hpp"

namespace Application::Ports {

class IObservabilityToInfrastructurePort {
public:
    virtual ~IObservabilityToInfrastructurePort() = default;
    virtual void ingest(
        const DTO::SoilElectricalObservabilityDTO& observability,
        const DTO::HardwareFeasibilityDTO& feasibility,
        const DTO::MembraneEnvelopeDTO& envelope) = 0;
};

class IInfrastructureToFourthDimensionPort {
public:
    virtual ~IInfrastructureToFourthDimensionPort() = default;
    virtual void ingest(
        const DTO::InfrastructureResilienceEvidenceDTO& evidence,
        const DTO::MembraneEnvelopeDTO& envelope) = 0;
};

class IObservabilityToFourthDimensionPort {
public:
    virtual ~IObservabilityToFourthDimensionPort() = default;
    virtual void ingest(
        const DTO::ObservabilityVisibilityEvidenceDTO& evidence,
        const DTO::MembraneEnvelopeDTO& envelope) = 0;
};

} // namespace Application::Ports
