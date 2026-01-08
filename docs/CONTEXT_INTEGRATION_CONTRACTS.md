# Context Integration Contracts

STATUS: living document (non-canonical; requires human validation)
SCOPE: integration rules for auxiliary contexts (read-only)
SOURCE OF TRUTH: `DDD_DiscursiveSystem_RecommendationTrajectory_STRATA.md`

-------------------------------------------------------------------------------
1. Purpose
-------------------------------------------------------------------------------

Define strict integration rules between auxiliary contexts to prevent
epistemological contamination (causality, decision authority, or simulation
feedback) while enabling comparative observation.

-------------------------------------------------------------------------------
2. Allowed Dependencies (Unidirectional)
-------------------------------------------------------------------------------

DiscursiveSystemContext depends on:
- NarrativeObservationContext (textual sources and trajectories)
- FourthDimensionSystem (temporal comparison only)

RecommendationTrajectoryContext depends on:
- NarrativeObservationContext
- FourthDimensionSystem
- PatchTrajectoryAnalysis

-------------------------------------------------------------------------------
3. Prohibited Dependencies
-------------------------------------------------------------------------------

- Core Domain
- Land Use
- Biophysical simulation
- Automated feedback loops

-------------------------------------------------------------------------------
4. Data Transfer Membranes (DTOs)
-------------------------------------------------------------------------------

DiscursiveSystemContext DTOs:
- `docs/DTO_DiscursiveSystem.txt`

RecommendationTrajectoryContext DTOs:
- `docs/DTO_RecommendationSnapshot.txt`
- `docs/DTO_RecommendationTrajectory.txt`

Rules:
- DTOs carry only observational data.
- No DTO may embed causal inference, decision outputs, or simulation parameters.
- Every DTO must preserve explicit SourceReference fields.

-------------------------------------------------------------------------------
5. Allowed Uses (Across Contexts)
-------------------------------------------------------------------------------

- Read-only visualization in UI
- Parallel comparison with temporal or spatial trajectories
- Corpus-level cataloging and indexing

-------------------------------------------------------------------------------
6. Forbidden Uses (Across Contexts)
-------------------------------------------------------------------------------

- Decision-making or management directives
- Automatic feedback to Core Domain or simulation
- Rewriting or normalizing narrative content

-------------------------------------------------------------------------------
7. LLM Assistance Boundary
-------------------------------------------------------------------------------

Allowed:
- Extraction and organization of candidate DTOs
- Highlighting contradictions and discursive absences

Forbidden:
- Authoring official system states
- Validating hypotheses
- Assigning causality
- Producing scientific conclusions

All LLM output is non-authoritative support and must be reviewed.
