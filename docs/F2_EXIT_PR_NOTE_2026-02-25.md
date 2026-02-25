# F2 Exit Candidate PR Note

Date: 2026-02-25  
Scope: F2 hardening closure evidence

## Summary

F2 hardening is finalized as an exit-candidate with canonical CI closure evidence.

- Invariants are fully mapped in `docs/INVARIANT_MATRIX.md` with ADR source, executable evidence, merge-blocking gate, and owner role.
- Determinism stability window is now merge-blocking (`enforce`) in CI.
- Canonical closure run is `STRATA-CI #48` (`9a699ce`) with all jobs green:
  - https://github.com/jpereiratrindade/SisterSTRATA/actions/runs/22400154856

## ADR / Governance References

- `adr/ADR-000_Architecture_Governance.md`
- `adr/ADR-001_Infrastructure_as_Evidence_Axis.md`
- `adr/ADR-002_Deterministic_Core_Non_Negotiable_Constraint.md`
- `adr/ADR-004_Integration_Contract_Observability_Infrastructure_FourthDimension.md`
- `adr/ADR-005_Scientific_Hardening_Phase_F1.md`
- `adr/ADR-007_Canonical_Versioning_Contract_for_Engine_and_Scientific_Model.md`
- `docs/F2_HARDENING_CHECKPOINT_2026-02-25.md`

## Versioning Declaration

- Engine version target: `1.10.6`
- Scientific model version target: `1.0.0`
- Scientific breaking change declared: `no`

## Validation Evidence

- Local suite: `./scripts/run_f2_headless_suite.sh` (all gate groups pass)
- CI closure evidence: run `#48` (all jobs success)
- Determinism stability window: `samples=20`, `uniqueHashes=1`, `promotion_ready=1`, mode `enforce`

## Minimal release checklist

- [x] Versioning (engine + scientific) declared
- [x] No scientific breaking change without explicit policy
- [x] Deterministic gates passing
- [x] Cross-runner determinism compare passing
- [x] Release stability criteria satisfied for F2 closure
- [x] Governance/version alignment gates passing
