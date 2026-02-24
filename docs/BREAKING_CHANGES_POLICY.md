# Breaking Changes Policy

Status: Active  
Owner: Technical Lead  
Co-owner: Scientific Governance Lead  
Review cadence: Quarterly or on policy exception

## 1. Purpose

Define what constitutes a breaking change and what evidence is required before merge.

## 2. Breaking Change Types

### Software Breaking Change

A change that breaks compatibility of APIs, schemas, or integrations.

Typical cases:
- DTO/JSON schema incompatible updates,
- removed or renamed public fields/endpoints,
- incompatible build/runtime contract changes.

### Scientific Breaking Change

A change is scientific-breaking when, for identical input/config/seed, output differs in scientific meaning, metric semantics, or interpretation behavior.

Typical cases:
- changed metric formula or threshold semantics,
- changed ecological domain rule with output interpretation impact,
- changed classification ontology affecting comparable historical runs.

## 3. Mandatory Actions for Scientific Breaking Change

All items below are mandatory:

1. Bump `scientificModelVersion` MAJOR in `docs/SCIENTIFIC_MODEL_VERSION.json`.
2. Add explicit note to `CHANGELOG.md` under a "Scientific Breaking Changes" section.
3. Reference ADR decision (new ADR or update status/links of existing ADR).
4. Provide migration/reproducibility note describing historical comparability impact.
5. Keep deterministic tests passing with updated expected baselines and rationale.

## 4. Merge Gate Rules

A PR must fail governance validation when any condition below is true:
- checklist item "Nenhum breaking change sem documentacao explicita" is unchecked,
- scientific breaking change is identified without MAJOR bump,
- no changelog scientific note for declared scientific breaking change,
- missing ADR reference for scientific-breaking decision.

## 5. Exception Process

Policy exceptions are allowed only when both owners approve:
- Technical Lead
- Scientific Governance Lead

Exception approval must be documented in the PR discussion and linked in commit message/tracking notes.

