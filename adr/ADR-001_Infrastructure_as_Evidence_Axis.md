# ADR-001: Infrastructure as Evidence Axis

Status: Accepted  
Date: 2026-02-24  
Decision Type: Domain boundary and ontology  
Applies to: STRATA Core v0.x

## 1. Context

STRATA integrates infrastructural systems including:

- FocinhoTrack (FT): animal biometric identification.
- SETO: Soil Electrical Trajectory Observatory.

These systems generate real-world signals that can inform analytical processes.

An architectural ambiguity existed regarding whether infrastructure components may directly mutate ecological state within core domain contexts such as:

- `IdentityResilienceContext`
- `SharedEnergyContext`

Unrestricted coupling would introduce:

- ontological contamination,
- hidden causal loops,
- epistemic ambiguity,
- non-deterministic simulation behavior.

A structural boundary is required to preserve scientific and architectural integrity.

## 2. Decision

Infrastructure components SHALL function exclusively as Evidence Providers.

They MAY:

- generate observational data,
- feed the Observational Layer,
- support analytical modules,
- inform recommendation systems.

They SHALL NOT:

- directly mutate ecological state,
- bypass domain validation layers,
- inject non-deterministic transitions into simulation core,
- modify bounded context state outside domain-defined rules.

All ecological state transitions MUST occur within domain contexts under explicit rule definition.

This decision is fundamental and establishes Infrastructure as epistemically external to ecological core.

## 3. Alternatives Considered

Alternative A: Full direct integration.  
Rejected due to ontological collapse risk and determinism instability.

Alternative B: Controlled runtime hooks.  
Rejected due to long-term epistemic ambiguity and architectural fragility.

Alternative C: Evidence axis model.  
Selected for structural clarity, scalability, and scientific defensibility.

## 4. Consequences

Positive:

- clear separation of concerns,
- preserved deterministic guarantees,
- hardware independence of simulation core,
- improved reproducibility and testability,
- stronger epistemic transparency.

Negative:

- additional validation layers required,
- slower feedback loops between hardware and domain,
- higher architectural discipline required.

## 5. Ontological Impact

Affected contexts:

- `InfrastructureLayer`
- Observational Layer
- `IdentityResilienceContext`
- `SharedEnergyContext`

Permitted causal flow:

`Infrastructure -> Observation -> Analysis -> Recommendation`

Forbidden causal flow:

`Infrastructure -> Direct Domain Mutation`

This ADR establishes Infrastructure as an evidence axis, not a causal axis.

## 6. Epistemic Risk

Violation may cause:

- confusion between measurement and causation,
- hardware signals treated as ecological truth,
- silent feedback loops,
- loss of scientific defensibility.

This ADR prevents collapse between modeled reality and measured reality.

## 7. Determinism Impact

This decision reinforces deterministic guarantees.

Infrastructure-generated evidence must be:

- logged,
- timestamped,
- versioned,
- explicitly injected into analytical processes.

Simulation core remains seed-deterministic and reproducible bit-by-bit under fixed seed.

## 8. Implementation Contract

All future FT/SETO integrations must:

- respect Observational Layer boundaries,
- avoid direct state mutation in core contexts,
- undergo ADR review if causal coupling is proposed.

Any proposal toward cyber-physical coupling requires a new ADR and major governance revision.

## 9. References

- `STRATA_Governance_Model_v0_1.md`
- `STRATA_DDD_InfrastructureLayer_v0_1.md`
