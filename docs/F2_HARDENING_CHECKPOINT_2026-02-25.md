# F2 Hardening Checkpoint

Status: Exit-candidate validated by CI (human sign-off pending)  
Date: 2026-02-25  
Scope: SisterSTRATA `main` (checkpoint state on 2026-02-25)

## 1. Executive Status

F2 is in late-stage hardening with broad executable coverage across:
- cross-context isolation,
- deterministic replay across runners,
- governance/versioning contracts,
- CI and local headless preset standardization.

Current phase status: **F2 formally closed in CI, pending final human governance sign-off**.

Canonical CI closure evidence:
- `STRATA-CI` run `#48` (commit `9a699ce`) completed with success in all jobs:
  - https://github.com/jpereiratrindade/SisterSTRATA/actions/runs/22400154856

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
Status: **Achieved**

Evidence:
- Enforced/merge-blocking gates:
  - application contracts,
  - core scientific suites,
  - static guards (`MembraneDependencyGuard`, `CoreDomainBoundaryGuard`, `CoreDeterminismPrimitiveGuard`, `ExceptionBoundaryGuard`, `GitArtifactHygieneGuard`, `VersionAlignmentGuard`),
  - cross-runner determinism compare.
- Determinism stability window:
  - objective readiness confirmed with `samples=20`, `uniqueHashes=1`, `promotion_ready=1` in historical evaluation,
  - workflow mode fixed to merge-blocking `enforce`.

Relevant files:
- `.github/workflows/strata-ci.yml`
- `scripts/check_determinism_stability_window.py`
- Canonical CI run: `STRATA-CI #48` (all gates green)

### Criterion 4. Owner + review cadence declared
Status: **Achieved**

Evidence:
- Matrix-level owner and review cadence exist in `docs/INVARIANT_MATRIX.md`.
- Per-invariant owner assignment is explicitly materialized via the `Owner` column in `docs/INVARIANT_MATRIX.md`.

## 3. Recent F2 Hardening Evidence (Commits)

- `a56c83c` test: add interleaved ingestion-reload cycle isolation guard
- `bf1c853` docs: assign invariant owners and promote f2 to exit-candidate
- `1251009` docs: add formal f2 hardening checkpoint
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

CI validation snapshot (UTC): `2026-02-25T14:06:35Z`
- Run: `STRATA-CI #48`
- Status: `success`
- Scope: `f1-hardening`, `f2-determinism-replay (ubuntu-22.04/24.04)`, `f2-determinism-compare`, `f2-determinism-stability-window`

## 5. Post-F2 Follow-ups

1. Optional stress expansion:
   - concurrent high-volume ingestion/reload stress family (if required for F2 closure policy).

## 6. Decision Recommendation

Recommended current declaration:
- **F2 = "Measurably Hardened (Formally Closed)"**
- Continue post-F2 follow-ups as incremental hardening workstreams.

## 7. PR/Release Summary Template

Use this text as a PR/release summary:

```
F2 hardening checkpoint finalized as exit-candidate with canonical CI evidence.

- All invariant classes in docs/INVARIANT_MATRIX.md are mapped to ADR + test/evidence + merge-blocking gate + owner.
- Determinism stability window is enforced in merge-blocking mode (`enforce`).
- Canonical closure evidence: STRATA-CI run #48 (commit 9a699ce), all jobs green.

Status declaration: F2 = Measurably Hardened (Formally Closed), pending final human governance sign-off.
```
