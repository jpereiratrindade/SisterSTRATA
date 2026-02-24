# ADR-001: Infrastructure as Transversal Evidence Axis (not decision axis)

Status: Proposed  
Date: 2026-02-24  
Decision Type: Domain boundary and ontology

## 1. Context

The project already formalizes:

- `InfrastructureLayer` with FT (`IdentityResilienceContext`), SETO (`SoilElectricalResilienceContext`) and `SharedEnergyContext`.
- explicit causality limits: infrastructure does not directly mutate ecological states.
- `ScientificInstrumentationContext` for detectability and hardware simulation.

## 2. Decision

Adopt this architectural rule:

**Infrastructure is a transversal axis of evidence, not a transversal axis of decision.**

Interpretation:

- Infrastructure measures, observes and reports operational performance.
- Observability derives instrumental requirements and hardware feasibility.
- Trajectory/Resilience consumes evidence artifacts as read-only inputs.
- No instrumental result may inject automatic decision into Core Domain.

## 3. Context boundaries

### 3.1 Observability (`ScientificInstrumentationContext`)

Responsibilities:

- detectability requirements,
- hardware feasibility simulation,
- explicit instrumental limitations.

Must not:

- mutate ecological states,
- mutate Core simulation rules,
- decide territorial management.

### 3.2 Infrastructure (`InfrastructureLayer`)

Responsibilities:

- FT/SETO operational resilience,
- territorial shared energy constraints,
- reliability and data-loss reporting.

Must not:

- define biophysical causality,
- create automatic simulation feedback loops.

### 3.3 Trajectory/Resilience (`Fourth Dimension`)

Responsibilities:

- temporal trajectory reading,
- coherence/stability/reorganization interpretation,
- resilience as emergent property.

Receives infrastructure and instrumentation outputs only as observational evidence.

## 4. Integration rule

Allowed flow:

`Core Simulation -> Observability -> Infrastructure Reports -> Trajectory/Resilience (read-only)`

Forbidden flow:

`Hardware/Infrastructure -> automatic mutation of ecological state or Core causal rule`

## 5. Consequences and trade-offs

Benefits:

- preserves ontology separation between measurement and simulation,
- avoids ecological model contamination by instrumental noise,
- allows FT/SETO evolution without reducing Core autonomy.

Accepted trade-off:

- a phenomenon may be ecologically valid and still instrumentally invisible.
- detectability failure does not invalidate causal model, it only limits empirical observability.

## 6. Validation criteria

Revisit this ADR when:

1. `ScientificInstrumentationContext` is implemented in core runtime.
2. A new observability <-> Fourth Dimension contract is introduced.
3. Any automatic decision loop is proposed (requires a dedicated ADR).
