# Release Stability Criteria

Status: Active  
Owner: Release Engineering Lead  
Co-owner: Scientific Governance Lead  
Review cadence: Per release cycle

## 1. Purpose

Define objective criteria for release stability levels and merge/release readiness.

## 2. Stability Levels

### Experimental

No stability guarantees. Suitable for research-only branches and prototypes.

### Beta

Feature-complete for a scope, but still open to controlled corrections before stable release.

### Stable

Meets all mandatory engineering and scientific governance gates.

### LTS

Stable plus long-term maintenance commitment and frozen compatibility policy window.

## 3. Stable Release Gate (Mandatory)

A release is considered Stable only when all items below are true:

1. Scientific CI gates are green (`f1-hardening` plus governance validation).
2. Deterministic tests pass for fixed seed/config baselines.
3. `docs/SCIENTIFIC_MODEL_VERSION.json` is valid and consistent with release intent.
4. No undocumented breaking change (software or scientific).
5. PR checklist governance items are completed for merged release PRs.
6. ADR references are updated when architectural/scientific contracts changed.

## 4. Minimum Release Checklist

- Engine version declared.
- Scientific model version declared.
- Deterministic baselines validated.
- Breaking change status documented.
- ADR impact reviewed.
- CHANGELOG updated.

## 5. Promotion Rules

- Experimental -> Beta: baseline tests and docs in place for scoped feature set.
- Beta -> Stable: all mandatory stable gates satisfied.
- Stable -> LTS: explicit maintenance policy and compatibility window approved.

## 6. Determinism Stability Window Promotion (F2 -> Enforced Gate)

The historical determinism trend check (`f2-determinism-stability-window`) starts in monitoring mode (`warn`) and is promoted to merge-blocking (`enforce`) only when all criteria below are met:

1. At least 20 successful historical samples on `main` are available (window size >= 20).
2. No hash divergence in the full window (`uniqueHashes == 1`).
3. No internal artifact inconsistency in any sampled run.
4. No CI infrastructure incident flagged for determinism jobs in the same observation window.
5. Scientific Governance Lead + Release Engineering Lead approve the mode switch in PR notes.

Operational parameters are configured in workflow env:

- `STRATA_DETERMINISM_STABILITY_MODE`
- `STRATA_DETERMINISM_STABILITY_WINDOW`
- `STRATA_DETERMINISM_STABILITY_MIN_SAMPLES`
