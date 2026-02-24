# ADR-002: Integration Contract for Observability -> Infrastructure -> Fourth Dimension

Status: Proposed  
Date: 2026-02-24  
Decision Type: Cross-context integration contract

## 1. Context

ADR-001 defines the evidence-axis principle.  
This ADR defines the technical membrane contract for data exchange.

## 2. Decision

Adopt a read-only integration contract with three membranes:

1. Observability -> Infrastructure
2. Infrastructure -> Fourth Dimension
3. Observability -> Fourth Dimension (optional, derived)

Central rule:

- only observational artifacts can cross these membranes,
- no membrane payload may carry decision commands, automatic causal inference, or parameters that mutate Core Domain.

## 3. Membrane 1: Observability -> Infrastructure

Purpose:

- provide detectability limits and hardware feasibility envelope.

Allowed artifacts:

- `SoilElectricalObservability.latest.json`
- `HardwareFeasibility.latest.json`

Conceptual minimum fields:

`SoilElectricalObservability`:
- `transitionType`
- `deltaResistivity`
- `snrRequired`
- `recommendedFrequencyBand`

`HardwareFeasibility`:
- `adcResolutionBits`
- `achievableSNR`
- `minimumDetectableDelta`
- `meetsRequirement`
- `limitingFactor`

Restrictions:

- no executable action directives,
- no mutation of `EnergyAllocationPolicy`,
- no ecological or productive state mutation.

## 4. Membrane 2: Infrastructure -> Fourth Dimension

Purpose:

- add operational reliability evidence to temporal interpretation.

Allowed artifact:

- `InfrastructureResilience.latest.json`

Conceptual consumable fields:

- `runConfig.ecologicalScenario`
- `finalState.poolStorageWh`
- `finalState.identity.reliabilityIndex`
- `finalState.identity.requestedWh/allocatedWh/consumedWh`
- `finalState.soil.reliabilityIndex`
- `finalState.soil.requestedWh/allocatedWh/consumedWh`

Restrictions:

- Fourth Dimension consumes as observational layer only,
- no rewriting infrastructure reports,
- no automatic feedback to biophysical simulation.

## 5. Membrane 3: Observability -> Fourth Dimension (optional)

Purpose:

- compare ecological trajectory with instrumental detectability envelope.

Allowed use:

- visibility/invisibility comparative reading,
- observational uncertainty signaling.

Forbidden use:

- automatic invalidation of ecological trajectory,
- translating instrumental invisibility into absence of phenomenon.

## 6. Domain events policy

Allowed observational events:

- `DetectabilityRequirementGenerated`
- `HardwareConfigurationTested`
- `HardwareLimitationIdentified`
- `InfrastructureResilienceReported`
- `InfrastructureEvidenceAttachedToTrajectory`

Rules:

- events are publish-only observational facts,
- no command semantics,
- no ecological state mutation.

## 7. Dependency contract

Allowed unidirectional dependencies:

- `Core Simulation -> ScientificInstrumentationContext`
- `ScientificInstrumentationContext -> InfrastructureLayer` (artifact-based)
- `InfrastructureLayer -> Fourth Dimension` (artifact-based)
- `ScientificInstrumentationContext -> Fourth Dimension` (optional, artifact-based)

Forbidden:

- `ScientificInstrumentationContext -> Core Simulation mutation`
- `InfrastructureLayer -> Core Simulation mutation`
- circular dependencies among the three contexts.

## 8. DTO and event guardrails

Every cross-membrane DTO/event must include:

- `type = observational_evidence`
- `causalInterpretationAllowed = false`
- `decisionDirective = null`
- explicit `sourceReference` (context + artifact + timestamp)

Non-compliant payloads must be rejected by integration adapters.

## 9. Consequences and trade-offs

Benefits:

- preserves causal integrity of Core Domain,
- enables hardware/observability evolution with governance,
- keeps Fourth Dimension interpretive, not control-oriented.

Trade-offs:

- stricter contract discipline,
- human validation required before promoting observational fields into consolidated indicators.

## 10. Validation criteria

Move this ADR from `Proposed` to `Accepted` when:

1. explicit DTOs/ports exist for the three membranes,
2. tests prevent automatic feedback to Core Simulation,
3. human validation confirms minimum payload viability in workspace flows.
