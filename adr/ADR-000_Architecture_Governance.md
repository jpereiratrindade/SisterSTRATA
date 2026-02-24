# ADR-000: Architecture Governance Model

Status: Accepted  
Date: 2026-02-24  
Decision Type: Foundational governance

## 1. Context

SisterSTRATA has reached a stage where architectural choices affect:

- domain boundaries,
- epistemological safeguards,
- long-term maintainability,
- scientific reproducibility.

Architectural decisions must stop depending on author memory and become explicit project history.

## 2. Decision

Adopt ADRs as a systematic discipline for any relevant architectural decision.

This is not optional guidance. It is a governance rule.

## 3. When an ADR is mandatory

Create an ADR when at least one condition is true:

1. A new `Bounded Context` is introduced.
2. A context changes responsibilities.
3. A layer gains or loses causal power.
4. A transversal integration contract is defined or changed.
5. An ontological principle is formalized or revised.
6. A macro-domain is introduced (for example, instrumentation contexts).

ADR is optional for local refactors with no boundary, causality, or contract impact.

## 4. Repository structure

Canonical location:

`/adr`

Naming convention:

`ADR-<NNN>_<Short_Title>.md`

Examples:

- `ADR-000_Architecture_Governance.md`
- `ADR-001_Infrastructure_as_Evidence_Axis.md`

## 5. Lifecycle and status

Allowed statuses:

- `Proposed`
- `Accepted`
- `Rejected`
- `Superseded`
- `Deprecated`

Rules:

- Start as `Proposed` unless explicitly approved.
- Move to `Accepted` only after human validation.
- Never delete ADRs.
- If replaced, keep old file and mark it `Superseded` with pointer to the newer ADR.

## 6. Minimum ADR template

Every ADR must include:

1. Context
2. Decision
3. Scope / boundaries
4. Allowed and prohibited flows (if integration-related)
5. Consequences and trade-offs
6. Validation criteria

## 7. Authority and change control

- Any collaborator can propose an ADR.
- Acceptance requires human maintainers approval.
- Code and docs that violate an `Accepted` ADR are non-compliant by default.

## 8. Consequences

Benefits:

- explicit architectural memory,
- less ontology drift,
- objective conflict resolution,
- stronger scientific and engineering governance.

Cost:

- slight process overhead, accepted as long-term quality investment.
