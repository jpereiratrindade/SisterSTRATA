# ADR Index

This directory is the canonical registry for Architecture Decision Records (ADRs) in SisterSTRATA.

Rules:

- ADRs are immutable historical records.
- ADR files are numbered sequentially: `ADR-000`, `ADR-001`, ...
- ADRs are never deleted.
- If replaced, mark status as `Superseded` and reference the newer ADR.
- Keep one ADR per architectural decision.

Status vocabulary:

- `Proposed`
- `Accepted`
- `Rejected`
- `Superseded`
- `Deprecated`

Current ADRs:

- [ADR-000_Architecture_Governance.md](ADR-000_Architecture_Governance.md)
- [ADR-001_Infrastructure_as_Evidence_Axis.md](ADR-001_Infrastructure_as_Evidence_Axis.md)
- [ADR-002_Deterministic_Core_Non_Negotiable_Constraint.md](ADR-002_Deterministic_Core_Non_Negotiable_Constraint.md)
- [ADR-003_Infrastructure_Evidence_Axis_Superseded_Draft.md](ADR-003_Infrastructure_Evidence_Axis_Superseded_Draft.md)
- [ADR-004_Integration_Contract_Observability_Infrastructure_FourthDimension.md](ADR-004_Integration_Contract_Observability_Infrastructure_FourthDimension.md)
- [ADR-005_Scientific_Hardening_Phase_F1.md](ADR-005_Scientific_Hardening_Phase_F1.md)
- [ADR-006_Domain_Naming_Disambiguation_Soil_vs_SETO.md](ADR-006_Domain_Naming_Disambiguation_Soil_vs_SETO.md)

Normative governance reference:

- [`STRATA_Governance_Model_v0_1.md`](../STRATA_Governance_Model_v0_1.md)

ADR metadata extraction (analysis layer, read-only):

- Command: `python3 scripts/build_adr_catalog.py`
- Outputs:
  - `reports/architecture/ArchitectureDecisionIndex.latest.json`
  - `reports/architecture/ArchitectureDecisionIndex.latest.md`
- Optional timestamped outputs:
  - `python3 scripts/build_adr_catalog.py --with-stamped`
