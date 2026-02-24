# Scientific Model Versioning Policy

Status: Active  
Owner: Scientific Governance Lead  
Review cadence: Quarterly or on scientific breaking change

## 1. Purpose

Define versioning rules for scientific behavior so STRATA outputs remain auditable and reproducible across releases.

Normative references:
- `adr/ADR-002_Deterministic_Core_Non_Negotiable_Constraint.md`
- `adr/ADR-004_Integration_Contract_Observability_Infrastructure_FourthDimension.md`
- `adr/ADR-005_Scientific_Hardening_Phase_F1.md`

## 2. Versioning Layers

STRATA uses two mandatory layers:

1. Engine Version (`engineVersion`): software lifecycle using SemVer.
2. Scientific Model Version (`scientificModelVersion`): scientific semantics lifecycle using SemVer.

Optional subcomponents can be versioned in `componentVersions` (for example `fourthDimensionModel`, `observationalContracts`).

The canonical machine-readable source is:
- `docs/SCIENTIFIC_MODEL_VERSION.json`

## 3. Scientific SemVer Rules

### MAJOR bump

Required when same input/config/seed can produce a scientifically different interpretation or metric meaning.

Examples:
- metric definition changes,
- domain rule changes with scientific impact,
- ontology/contract changes that alter interpretation.

### MINOR bump

Required for additive, backward-compatible scientific capabilities.

Examples:
- new optional scientific indicator,
- new optional observational field with no semantic change to existing outputs.

### PATCH bump

Required for corrections that do not alter scientific meaning.

Examples:
- documentation clarifications,
- implementation fixes preserving accepted scientific behavior and deterministic baselines.

## 4. Runtime and Artifact Requirements

Every scientific execution artifact must carry, at minimum:
- `engineVersion`
- `scientificModelVersion`
- deterministic metadata already required by ADR-002 (`seed`, `tier`, `entropySources`, `stateHash`)

## 5. Governance Gate Requirements

For pull requests affecting scientific behavior:
- PR checklist item "Versioning (engine + scientific) preenchido" must be checked.
- `docs/SCIENTIFIC_MODEL_VERSION.json` must stay valid and SemVer-compliant.
- If scientific meaning changes, apply MAJOR bump and follow `docs/BREAKING_CHANGES_POLICY.md`.

