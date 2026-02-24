# Invariant Matrix

Status: Draft (requires human validation)  
Owner: Scientific Governance Lead  
Review cadence: Per release cycle

## 1. Purpose

Make architectural/scientific invariants explicit, auditable, and directly tied to automated enforcement.

This matrix complements:
- `docs/ADR_ENFORCEMENT_MATRIX_F1.md`
- `docs/ADR_STATUS_CODE_ALIGNMENT_2026-02-24.md`
- `docs/EXCEPTION_BOUNDARY_POLICY.md`
- `adr/ADR-000_Architecture_Governance.md`
- `adr/ADR-001_Infrastructure_as_Evidence_Axis.md`
- `adr/ADR-002_Deterministic_Core_Non_Negotiable_Constraint.md`
- `adr/ADR-004_Integration_Contract_Observability_Infrastructure_FourthDimension.md`
- `adr/ADR-005_Scientific_Hardening_Phase_F1.md`
- `adr/ADR-006_Domain_Naming_Disambiguation_Soil_vs_SETO.md`

## 2. Invariant -> Test -> Gate

| Invariant | Origin (ADR) | Risk if violated | Associated test/evidence | Associated CI workflow gate | Status |
| --- | --- | --- | --- | --- | --- |
| Infrastructure cannot mutate ecological core state or observational state across context boundaries. | ADR-001, ADR-004 | Ontological contamination, hidden causal loops, invalid scientific interpretation. | `tests/application/MembraneContractEnforcementTest.cpp` (`InfrastructureRunCannotMutateEcologicalCoreState`, `InfrastructureRunPreservesCrossContextStateSnapshots`, `ValidObservationalIngestionThenInfrastructureRunPreservesCrossContextState`, `SessionIngestionRejectsInvalidMembraneEnvelopeAndPreservesState`) | `.github/workflows/strata-ci.yml` (`ApplicationMapperTest`) | Protected |
| Membrane payloads must be read-only, no decision directives, no causal semantics crossing contexts. | ADR-004 | Cross-context command injection and invalid causal authority. | `tests/application/MembraneContractEnforcementTest.cpp` (`RejectsDecisionDirectiveAcrossMembrane`, `RejectsCausalSemanticsAcrossMembrane`) | `.github/workflows/strata-ci.yml` (`ApplicationMapperTest`) | Protected |
| Tier-1 determinism: same binary/config/seed must produce identical state hash; determinism metadata must be emitted; hash must match canonical deterministic payload. | ADR-002, ADR-005 | Non-reproducible scientific outputs, audit failure. | `tests/application/InfrastructureResilienceRunTest.cpp` (`SameSeedSameConfigSameHash`, `ReportContainsDeterminismMetadata`, `Tier1RequiresNonZeroSeed`, `StateHashMatchesCanonicalPayloadAndReplayPayloadIsIdentical`), `src/application/Session.cpp` (`stateHash`, deterministic payload) | `.github/workflows/strata-ci.yml` (`ApplicationMapperTest`) | Protected |
| Core domain must not depend on outer layers (application/ui/world3d/observational/infrastructure adapters). | ADR-000, ADR-005 | Boundary erosion and architecture drift. | `scripts/check_core_domain_dependencies.sh` | `.github/workflows/strata-ci.yml` (`CoreDomainBoundaryGuard`) | Protected |
| Infrastructure/Fourth-Dimension dependency membrane must remain unidirectional. | ADR-001, ADR-004 | Forbidden coupling across bounded contexts. | `scripts/check_membrane_dependencies.sh` | `.github/workflows/strata-ci.yml` (`MembraneDependencyGuard`) | Protected |
| Scientific versioning metadata and release governance artifacts must remain valid and complete. | ADR-000, ADR-005 (+ policy docs) | Inconsistent scientific versioning and weak release traceability. | `scripts/validate_governance.sh`, `docs/SCIENTIFIC_MODEL_VERSION.json` schema checks | `.github/workflows/strata-ci.yml` (`Run Governance Gates`) | Protected |
| Core scientific algorithms (energy/hydro domain) must keep deterministic and expected behavior. | ADR-002, ADR-005 | Silent scientific regressions in core domain behavior. | `tests/core/EnergyAllocationPolicyTest.cpp`, `tests/core/HydroDomainTest.cpp` | `.github/workflows/strata-ci.yml` (`CoreEnergyAllocationPolicyTest`, `CoreHydroDomainTest`) | Protected |
| Broad exception shields (`catch(...)`) are allowed only at explicit runtime boundaries (`main`, world3d callback). | ADR-000, ADR-005 (+ `docs/EXCEPTION_BOUNDARY_POLICY.md`) | Silent failure swallowing in core/application/infrastructure flows. | `scripts/check_exception_boundaries.sh` | `.github/workflows/strata-ci.yml` (`ExceptionBoundaryGuard`) | Protected |
| Cross-context infrastructure runs must preserve energy safety invariants (non-negative pool, `consumed <= allocated <= requested`, reliability in `[0,1]`). | ADR-001, ADR-002, ADR-004 | System-level regressions not visible in isolated module tests. | `tests/application/InfrastructureResilienceRunTest.cpp` (`CrossContextEnergyInvariantsHoldForSevereDrought`) | `.github/workflows/strata-ci.yml` (`ApplicationMapperTest`) | Protected |

## 3. Current Residual Gaps

1. Build profile standardization was introduced with `CMakePresets.json`, but team adoption in local/dev/CI scripts still needs consolidation.
2. Scenario-level enforcement currently focuses on infrastructure resilience flow; additional cross-context scenario families are still pending (for example narrative/observational coupling constraints).
3. Determinism enforcement is currently Tier-1 (same binary/config/seed); cross-runner replay compare in CI is progressive (`warn` in PR, `enforce` on `main`).

## 4. F2 Exit Hint

F2 can be considered "measurably hardened" when every invariant above has:
1. explicit source ADR,  
2. executable test/evidence,  
3. merge-blocking CI gate,  
4. owner + review cadence in this matrix.
