# ADR-005: Establish Scientific Hardening Phase (F1)

Status: Accepted  
Date: 2026-02-24  
Decision Type: Institutional hardening phase and execution policy  
Scope: STRATA Core v0.x and architectural governance

## 1. Context

STRATA already has accepted architectural commitments for:

- governance discipline (`ADR-000`),
- infrastructure as evidence axis (`ADR-001`),
- deterministic core constraint (`ADR-002`).

However, not all of these commitments are fully enforced by executable safeguards.
`ADR-004` remains `Proposed` and requires implementation-level enforcement.

To transition STRATA from research-grade evolution to scientific infrastructure operation,
an explicit hardening phase is required.

## 2. Decision

STRATA enters **F1 - Scientific Hardening** as an institutional phase.

From this ADR onward:

1. Any change that violates `ADR-001` or `ADR-002` is classified as an **architectural regression**, and must be detectable by automated tests linked to the corresponding ADR invariant.
2. During F1, no new domain functionality is introduced into Core Domain.
3. FT/SETO evolution is restricted to hardening work (determinism, membrane, traceability).
4. `ADR-004` remains `Proposed` until automated tests prove membrane enforcement, including explicit failing cases for forbidden `infrastructure -> ecological state mutation` feedback.

## 3. Freeze Policy (F1)

During F1:

- No new core modules.
- No new 4D feature expansion.
- No deep SETO integration into ecological causal state.
- No bypass of current bounded-context contracts.

Core Domain (for freeze policy) includes:

- ecological domain simulation logic,
- infrastructure domain logic,
- session orchestration and execution flow,
- state/report generation contracts used by runtime execution.

Edge experimental scope (outside core freeze) includes:

- isolated prototypes not integrated into runtime core contracts,
- exploratory FT/SETO experiments without causal integration to core domain state.

Allowed work:

- determinism enforcement,
- membrane enforcement,
- reproducibility metadata/signature,
- automated test gates and CI checks.

## 4. Determinism Baseline for F1

F1 adopts **Tier 1 determinism**:

- same binary build,
- same configuration,
- same seed,

must produce identical deterministic state hash.

Tier 1 does **not** guarantee cross-machine or cross-compiler determinism.
Cross-machine bit-level determinism is not required in F1.

## 5. Exit Criteria (F1 Completion)

F1 is complete only when all criteria are met:

1. Deterministic replay test passes with identical state hash under same seed/config.
2. Determinism metadata is emitted per execution (seed, tier, entropy sources).
3. Membrane violation triggers automated test failure.
4. Scientific signature is emitted per execution with traceability metadata.
5. `ADR-004` acceptance criteria are satisfied and validated.
6. Criteria 1-5 are enforced in CI with merge-blocking gates.

## 6. Consequences

Positive:

- stronger architectural integrity,
- reduced ontology drift,
- auditable scientific operation model,
- improved institutional readiness.

Trade-off:

- temporary reduction in feature velocity during F1 window.

## 7. References

- `adr/ADR-000_Architecture_Governance.md`
- `adr/ADR-001_Infrastructure_as_Evidence_Axis.md`
- `adr/ADR-002_Deterministic_Core_Non_Negotiable_Constraint.md`
- `adr/ADR-004_Integration_Contract_Observability_Infrastructure_FourthDimension.md`
- `STRATA_Governance_Model_v0_1.md`
