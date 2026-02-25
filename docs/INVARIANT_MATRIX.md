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
- `adr/ADR-007_Canonical_Versioning_Contract_for_Engine_and_Scientific_Model.md`

## 2. Invariant -> Test -> Gate

| Invariant | Origin (ADR) | Risk if violated | Associated test/evidence | Associated CI workflow gate | Status |
| --- | --- | --- | --- | --- | --- |
| Infrastructure cannot mutate ecological core state or observational state across context boundaries. | ADR-001, ADR-004 | Ontological contamination, hidden causal loops, invalid scientific interpretation. | `tests/application/CrossContextIsolationTest.cpp` (`InfrastructureRunCannotMutateEcologicalCoreState`, `InfrastructureRunPreservesCrossContextStateSnapshots`, `ValidObservationalIngestionThenInfrastructureRunPreservesCrossContextState`, `InvalidObservationalIngestionThenInfrastructureRunPreservesCrossContextState`, `InvalidBundleDirectoryIngestionThenInfrastructureRunPreservesCrossContextState`) + `tests/application/MembraneContractEnforcementTest.cpp` (`SessionIngestionRejectsInvalidMembraneEnvelopeAndPreservesState`) | `.github/workflows/strata-ci.yml` (`ApplicationMapper_CrossContextIsolation`, `ApplicationMapper_MembraneContract`) | Protected |
| Observational ingestion must not causally influence infrastructure deterministic outcomes for identical infrastructure config/seed/scenario. | ADR-001, ADR-002, ADR-004 | Hidden cross-context coupling and non-auditable causality from observational payloads into infrastructure output. | `tests/application/CrossContextIsolationTest.cpp` (`ObservationalIngestionDoesNotChangeInfrastructureDeterministicOutcome`) | `.github/workflows/strata-ci.yml` (`ApplicationMapper_CrossContextIsolation`) | Protected |
| Cognitive interpretation requests (LLM-assisted observational analysis) must not causally influence infrastructure deterministic outcomes for identical infrastructure config/seed/scenario. | ADR-001, ADR-002, ADR-004 | Hidden coupling from cognitive-assistance flows into infrastructure causality and reproducibility drift. | `tests/application/CrossContextIsolationTest.cpp` (`CognitiveInterpretationDoesNotChangeInfrastructureDeterministicOutcome`) | `.github/workflows/strata-ci.yml` (`ApplicationMapper_CrossContextIsolation`) | Protected |
| Trajectory impact profile generation (observational analytics) must not causally influence infrastructure deterministic outcomes for identical infrastructure config/seed/scenario. | ADR-001, ADR-002, ADR-004 | Hidden coupling from analytical read-models into infrastructure causality and reproducibility drift. | `tests/application/CrossContextIsolationTest.cpp` (`TrajectoryImpactProfileGenerationDoesNotChangeInfrastructureDeterministicOutcome`) | `.github/workflows/strata-ci.yml` (`ApplicationMapper_CrossContextIsolation`) | Protected |
| Membrane payloads must be read-only, no decision directives, no causal semantics crossing contexts. | ADR-004 | Cross-context command injection and invalid causal authority. | `tests/application/MembraneContractEnforcementTest.cpp` (`RejectsDecisionDirectiveAcrossMembrane`, `RejectsCausalSemanticsAcrossMembrane`) | `.github/workflows/strata-ci.yml` (`ApplicationMapper_MembraneContract`) | Protected |
| Tier-1 determinism: same binary/config/seed must produce identical state hash; determinism metadata must be emitted; hash must match canonical deterministic payload. | ADR-002, ADR-005 | Non-reproducible scientific outputs, audit failure. | `tests/application/InfrastructureResilienceRunTest.cpp` (`SameSeedSameConfigSameHash`, `ReportContainsDeterminismMetadata`, `Tier1RequiresNonZeroSeed`, `StateHashMatchesCanonicalPayloadAndReplayPayloadIsIdentical`), `src/application/Session.cpp` (`stateHash`, deterministic payload) | `.github/workflows/strata-ci.yml` (`ApplicationMapper_InfrastructureResilience`) | Protected |
| Determinism state hash should remain stable across a historical CI window on `main` (trend monitoring over recent successful runs). | ADR-002, ADR-005 | Undetected long-horizon drift despite per-run cross-runner equality. | `scripts/check_determinism_stability_window.py` (current hash + historical artifact window analysis) | `.github/workflows/strata-ci.yml` (`f2-determinism-stability-window`) | Monitored (warn mode) |
| Core domain must not depend on outer layers (application/ui/world3d/observational/infrastructure adapters). | ADR-000, ADR-005 | Boundary erosion and architecture drift. | `scripts/check_core_domain_dependencies.sh` | `.github/workflows/strata-ci.yml` (`CoreDomainBoundaryGuard`) | Protected |
| Infrastructure/Fourth-Dimension dependency membrane must remain unidirectional. | ADR-001, ADR-004 | Forbidden coupling across bounded contexts. | `scripts/check_membrane_dependencies.sh` (static include-level guard; direct dependencies) | `.github/workflows/strata-ci.yml` (`MembraneDependencyGuard`) | Protected |
| Scientific versioning metadata and release governance artifacts must remain valid and complete. | ADR-007 | Inconsistent scientific versioning and weak release traceability. | `scripts/validate_governance.sh`, `scripts/check_version_alignment.sh`, `docs/SCIENTIFIC_MODEL_VERSION.json` schema checks | `.github/workflows/strata-ci.yml` (`Run Governance Gates`, `VersionAlignmentGuard`) | Protected |
| Repository index must remain free of tracked build artifacts and transient binaries (`*.o`, build outputs). | ADR-005 | Polluted history, noisy diffs, accidental release contamination. | `scripts/check_git_artifact_hygiene.sh` | `.github/workflows/strata-ci.yml` (`GitArtifactHygieneGuard`) | Protected |
| Core scientific domain suites (soils/spatial_pattern/vegetation/fourth_dimension/energy/hydro/simulation) must keep deterministic and expected behavior. | ADR-002, ADR-005 | Silent scientific regressions in core domain behavior. | `tests/core/SoilRasterizerTest.cpp`, `tests/core/PatchAnalysisTest.cpp`, `tests/core/PatchTrajectoryTest.cpp`, `tests/core/TestTrajectoryLOD.cpp`, `tests/core/VegetationSystemVectorTest.cpp`, `tests/core/SlopeFixTest.cpp`, `tests/core/EnergyAllocationPolicyTest.cpp`, `tests/core/HydroDomainTest.cpp`, `tests/core/EnvironmentControllerDeterminismTest.cpp`, `tests/core/HypothesisIdDeterminismTest.cpp` | `.github/workflows/strata-ci.yml` (`CoreSoilRasterizerTest`, `CorePatchAnalysisTest`, `CorePatchTrajectoryTest`, `CoreTrajectoryLODTest`, `CoreVegVectorTest`, `CoreSlopeFixTest`, `CoreEnergyAllocationPolicyTest`, `CoreHydroDomainTest`, `CoreEnvironmentControllerDeterminismTest`, `CoreHypothesisIdDeterminismTest`) | Protected |
| Core domain must avoid wall-clock and nondeterministic random primitives. | ADR-002, ADR-005 | Non-reproducible model behavior under identical configs/seeds and hidden cross-runner drift. | `scripts/check_core_determinism_primitives.sh` | `.github/workflows/strata-ci.yml` (`CoreDeterminismPrimitiveGuard`) | Protected |
| Broad exception shields (`catch(...)`) are allowed only at explicit runtime boundaries (`main`, world3d callback). | ADR-000, ADR-005 (+ `docs/EXCEPTION_BOUNDARY_POLICY.md`) | Silent failure swallowing in core/application/infrastructure flows. | `scripts/check_exception_boundaries.sh` | `.github/workflows/strata-ci.yml` (`ExceptionBoundaryGuard`) | Protected |
| Cross-context infrastructure runs must preserve energy safety invariants (non-negative pool, `consumed <= allocated <= requested`, reliability in `[0,1]`). | ADR-001, ADR-002, ADR-004 | System-level regressions not visible in isolated module tests. | `tests/application/InfrastructureResilienceRunTest.cpp` (`CrossContextEnergyInvariantsHoldForSevereDrought`) | `.github/workflows/strata-ci.yml` (`ApplicationMapper_InfrastructureResilience`) | Protected |

## 3. Current Residual Gaps

1. Build profile standardization was introduced with `CMakePresets.json`, but team adoption in local/dev/CI scripts still needs consolidation.
2. Scenario-level enforcement now includes observational/cognitive/impact-profile-to-infrastructure coupling protection; additional cross-context scenario families are still pending (for example project persistence side-effect constraints).
3. Determinism trend tracking was added in CI (`f2-determinism-stability-window`) in monitoring mode; promotion from `warn` to `enforce` follows the objective criteria in `docs/RELEASE_STABILITY_CRITERIA.md` (section "Determinism Stability Window Promotion").

## 4. F2 Exit Hint

F2 can be considered "measurably hardened" when every invariant above has:
1. explicit source ADR,  
2. executable test/evidence,  
3. merge-blocking CI gate,  
4. owner + review cadence in this matrix.
