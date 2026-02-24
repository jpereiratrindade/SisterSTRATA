# ADR-002: Deterministic Core as Non-Negotiable Constraint

Status: Accepted  
Date: 2026-02-24  
Decision Type: Determinism and reproducibility constraint  
Scope: STRATA Core v0.x

## 1. Context

STRATA is a rule-governed ecological simulation laboratory.

As system complexity increases, risks emerge:

- hidden entropy,
- time-based randomness,
- non-reproducible execution,
- divergent results across runs.

Scientific defensibility requires explicit determinism.

## 2. Determinism Tier Declaration

STRATA adopts Tier 1 logical determinism.

Under identical:

- binary build,
- configuration,
- version,
- seed,

simulation MUST produce identical results.

Cross-machine bit-level equivalence is not required at this stage.

## 3. Decision

The simulation core SHALL be deterministic.

All stochastic behavior MUST:

- use centralized RNG injection,
- declare entropy usage,
- use explicit seed,
- log seed in execution metadata.

Forbidden:

- implicit RNG initialization,
- time-based seed defaults,
- hardware entropy,
- uncontrolled concurrency ordering.

## 4. Ontological Impact

Simulation is defined as a rule-governed state transition system under controlled entropy.

Determinism is part of STRATA identity.

## 5. Future Evolution

Architecture must remain compatible with:

- Tier 2 cross-machine determinism,
- Tier 3 bit-level determinism.

Migration to higher tiers requires a new ADR.
