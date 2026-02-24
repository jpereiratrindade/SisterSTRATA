# ADR-006: Domain Naming Disambiguation for `soils` vs `soil` (SETO)

Status: Accepted  
Date: 2026-02-24  
Decision Type: Ubiquitous language and bounded-context naming

## 1. Context

The current core domain contains two similarly named directories with distinct meanings:

- `src/core/domain/soils`: pedological scientific model (`SiBCS`, `SoilSystem`).
- `src/core/domain/soil`: SETO instrumentation node (`SoilMonitorNode`), part of infrastructure evidence flow.

Although architecturally valid, the naming proximity introduces avoidable risk:

- include/import confusion for contributors,
- semantic drift in discussions and reviews,
- weaker bounded-context readability.

This is a language governance issue, not a scientific model change.

## 2. Decision

Adopt the following canonical naming:

1. Pedological scientific domain remains `soils`.
2. SETO instrumentation domain canonical name becomes `seto`.
3. `soil` is treated as legacy transitional naming only.

## 3. Migration Policy

Migration is incremental and non-disruptive:

1. Keep runtime behavior and scientific semantics unchanged.
2. Execute filesystem/include migration in controlled PRs.
3. Preserve compatibility shims only as temporary bridges.
4. Remove legacy `soil` naming after migration validation in CI.

## 4. Consequences

Positive:

- clearer bounded-context language,
- lower onboarding and maintenance friction,
- reduced naming-related architectural ambiguity.

Trade-off:

- temporary dual naming during migration window.

## 5. Versioning Note

This ADR changes naming contracts only.  
No scientific semantic change is introduced by this decision itself.

## 6. References

- `adr/ADR-000_Architecture_Governance.md`
- `adr/ADR-001_Infrastructure_as_Evidence_Axis.md`
- `adr/ADR-004_Integration_Contract_Observability_Infrastructure_FourthDimension.md`
- `docs/INVARIANT_MATRIX.md`
