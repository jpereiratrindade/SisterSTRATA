# F2 Hardening Checkpoint

Status: Draft (requires human validation)  
Date: 2026-02-25  
Scope: SisterSTRATA `main` up to commit `a56c83c`

## 1. Executive Status

F2 is in late-stage hardening with broad executable coverage across:
- cross-context isolation,
- deterministic replay across runners,
- governance/versioning contracts,
- CI and local headless preset standardization.

Current phase status: **F2 near-exit (not formally closed yet)**.

## 2. Exit Criteria Assessment

Reference baseline:
- `docs/INVARIANT_MATRIX.md` (section "F2 Exit Hint")
- `docs/RELEASE_STABILITY_CRITERIA.md` (section "Determinism Stability Window Promotion")

### Criterion 1. Explicit ADR source per invariant
Status: **Achieved**

Evidence:
- `docs/INVARIANT_MATRIX.md` maps invariants to ADR-000/001/002/004/005/007.

### Criterion 2. Executable test/evidence per invariant
Status: **Achieved**

Evidence:
- `tests/application/CrossContextIsolationTest.cpp` includes scenario families:
  - valid/invalid ingestion,
  - malicious sidecar rejection,
  - semantic poisoning schema-valid,
  - oversized payload,
  - escaping/unicode payloads,
  - permutation corpus (`csv`/`obj`),
  - seeded fuzz corpus,
  - transport-chain permutation,
  - long-session interleaving cycles.
- `tests/application/MembraneContractEnforcementTest.cpp`
- `tests/application/InfrastructureResilienceRunTest.cpp`
- `tests/core/*` deterministic/core scientific suites.

### Criterion 3. Merge-blocking CI gates
Status: **Partial**

Evidence:
- Enforced/merge-blocking gates:
  - application contracts,
  - core scientific suites,
  - static guards (`MembraneDependencyGuard`, `CoreDomainBoundaryGuard`, `CoreDeterminismPrimitiveGuard`, `ExceptionBoundaryGuard`, `GitArtifactHygieneGuard`, `VersionAlignmentGuard`),
  - cross-runner determinism compare.
- Determinism stability window:
  - active in `adaptive` mode (`warn` until promotion-ready, `enforce` when ready).
  - not yet permanently fixed as unconditional `enforce`.

Relevant files:
- `.github/workflows/strata-ci.yml`
- `scripts/check_determinism_stability_window.py`

### Criterion 4. Owner + review cadence declared
Status: **Partial**

Evidence:
- Matrix-level owner and review cadence exist in `docs/INVARIANT_MATRIX.md`.
- Per-invariant owner assignment is not yet explicitly materialized in table rows.

## 3. Recent F2 Hardening Evidence (Commits)

- `a56c83c` test: add interleaved ingestion-reload cycle isolation guard
- `dd8123c` test: add bundle-to-sidecar transport chain isolation guard
- `e9ab689` hardening: add seeded sidecar fuzz guard and adaptive determinism readiness
- `7ad15b0` test: add schema-valid permutation corpus isolation guard
- `b6e47b6` test: add escaping unicode sidecar deterministic isolation guard
- `645ad88` test: add oversized sidecar payload deterministic isolation guard
- `b9463bb` hardening: standardize local f2 headless preset workflow
- `3fd2afa` test: add schema-valid semantic sidecar poisoning isolation guard
- `3ed8fd8` hardening: reject malicious sidecar directives and add deterministic guard

## 4. Operational Validation Snapshot

Local execution snapshot (UTC): `2026-02-25T13:46:30Z`

Validated command:
- `./scripts/run_f2_headless_suite.sh`

Observed result:
- Application gates: pass
- Core scientific gates: pass
- Governance/static guards: pass

## 5. Remaining Work Before Formal F2 Closure

1. Determinism stability promotion governance:
   - keep collecting successful historical window samples,
   - confirm `promotion_ready=1` artifact in CI stability job,
   - execute formal approval step (Scientific Governance Lead + Release Engineering Lead),
   - optionally pin mode to permanent `enforce` once approved.

2. Ownership granularity:
   - add explicit per-invariant owner column (or companion owner map) to `docs/INVARIANT_MATRIX.md`.

3. Optional stress expansion:
   - concurrent high-volume ingestion/reload stress family (if required for F2 closure policy).

## 6. Decision Recommendation

Recommended current declaration:
- **F2 = "Measurably Hardened (Near Exit)"**
- **Not yet "Formally Closed"** until Criterion 3 (determinism stability promotion governance) and Criterion 4 (owner granularity) are closed by policy.
