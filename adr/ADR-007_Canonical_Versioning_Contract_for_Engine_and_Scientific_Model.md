# ADR-007: Canonical Versioning Contract for Engine and Scientific Model

Status: Accepted  
Date: 2026-02-25  
Decision Type: Governance contract for release/version traceability  
Scope: Engine versioning, scientific model versioning, and merge/release gates

## 1. Context

SisterSTRATA already has active policy documents for:

- scientific model versioning,
- breaking changes,
- release stability criteria.

These policies are enforced by CI guards (`validate_governance`, `check_version_alignment`) and PR governance checklist requirements.
However, the versioning policy itself still needs an explicit ADR-level canonical contract so that:

- versioning decisions are first-class architecture decisions,
- policy and enforcement stay synchronized over time,
- release traceability remains auditable as a non-optional governance commitment.

## 2. Decision

From this ADR onward, versioning governance is canonical and mandatory:

1. STRATA keeps two mandatory SemVer layers:
   - `engineVersion`: software/runtime lifecycle,
   - `scientificModelVersion`: scientific semantics lifecycle.
2. The machine-readable source of truth is `docs/SCIENTIFIC_MODEL_VERSION.json`.
3. `scientificModelVersion` bump semantics are mandatory:
   - `MAJOR`: scientific meaning or interpretation changes,
   - `MINOR`: additive backward-compatible scientific capabilities,
   - `PATCH`: corrections with no scientific semantic change.
4. Release metadata alignment is mandatory for `engineVersion` across:
   - `CMakeLists.txt`,
   - `Doxyfile`,
   - `README.md`,
   - `CHANGELOG.md`,
   - `docs/releases/vX.Y.Z.md`,
   - `docs/README.md` release index.
5. Pull requests affecting scientific behavior must document versioning intent and breaking-change status in the governance checklist.
6. CI governance gates that enforce this ADR are merge-blocking.

## 3. Non-Goals

This ADR does not:

- redefine deterministic tiers (covered by `ADR-002`),
- redefine membrane rules (covered by `ADR-004`),
- replace release engineering execution criteria (kept in policy docs).

## 4. Consequences

Positive:

- versioning intent becomes auditable at architecture level,
- lower risk of policy drift between docs and CI enforcement,
- clearer review protocol for scientific-impact changes.

Trade-off:

- higher process discipline required for version bumps and release metadata updates.

## 5. Enforcement Mapping

Primary automated enforcement:

- `scripts/validate_governance.sh`
- `scripts/check_version_alignment.sh`
- `.github/workflows/strata-ci.yml` (`Run Governance Gates`, `VersionAlignmentGuard`)

Normative policy documents (operational detail):

- `docs/SCIENTIFIC_MODEL_VERSIONING.md`
- `docs/BREAKING_CHANGES_POLICY.md`
- `docs/RELEASE_STABILITY_CRITERIA.md`

## 6. References

- `adr/ADR-000_Architecture_Governance.md`
- `adr/ADR-002_Deterministic_Core_Non_Negotiable_Constraint.md`
- `adr/ADR-005_Scientific_Hardening_Phase_F1.md`
- `docs/SCIENTIFIC_MODEL_VERSIONING.md`
- `docs/BREAKING_CHANGES_POLICY.md`
- `docs/RELEASE_STABILITY_CRITERIA.md`
