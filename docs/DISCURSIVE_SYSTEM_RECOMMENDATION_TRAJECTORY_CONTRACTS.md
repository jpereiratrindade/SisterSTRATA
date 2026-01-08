# Discursive System & Recommendation Trajectory: Contracts & Flows

STATUS: living document (non-canonical; requires human validation)
SCOPE: observational / declarative / analytical (read-only)
SOURCE OF TRUTH: `DDD_DiscursiveSystem_RecommendationTrajectory_STRATA.md`

-------------------------------------------------------------------------------
1. Purpose and Epistemic Contract
-------------------------------------------------------------------------------

These contexts register discursive systems and recommendation trajectories
around the territory without converting them into biophysical causality.

Architectural decision (explicit): no causal inference and no normative
automation. These contexts observe and organize statements; they do not decide.

-------------------------------------------------------------------------------
2. Discursive System Context (B)
-------------------------------------------------------------------------------

2.1 Core Objects (Ubiquitous Language)

Discursive System:
- Declared configuration of problems, actions, mechanisms, and expected effects
  expressed in textual sources.

Declared Problem / Action / Alleged Mechanism / Expected Effect:
- Atomic discursive claims extracted from sources, preserved as-is.

Source Reference:
- Pointer to the originating document, interview, questionnaire, or bulletin.

2.2 Invariants (Non-Negotiable)

- Discursive systems are not ecological systems.
- No factual causality is assumed or inferred.
- Every system remains explicitly linked to its source.
- Internal contradictions are valid states.
- Discursive absences are information, not errors.
- No element in this context alters the Core Domain.

2.3 Data Flow (Text to System)

Flow A: Ingestion
- Input: textual sources (interviews, documents, questionnaires, bulletins).
- Output: candidate discursive systems with explicit source references.
- Rule: preserve temporal context and interpretation metadata.

Flow B: Versioning
- Input: new or updated sources.
- Output: new discursive systems; never overwrite previous ones.
- Rule: coexistence is mandatory, including contradictions.

2.4 Interfaces (Conceptual, not code)

DiscursiveSystem
- DeclaredProblems
- DeclaredActions
- AllegedMechanisms
- ExpectedEffects
- SourceReferences
- TemporalContext
- InterpretationMetadata
- Immutable after creation

Aggregate: DiscursiveSystemRepository
- Versioned collection of DiscursiveSystems

-------------------------------------------------------------------------------
3. Recommendation Trajectory Context (C)
-------------------------------------------------------------------------------

3.1 Core Objects (Ubiquitous Language)

Recommendation Snapshot:
- A single recommendation with its context, intended action, expected outcome,
  and source reference, bound to a temporal context.

Recommendation Trajectory:
- Ordered sequence of snapshots, used for comparative analysis only.

3.2 Invariants (Non-Negotiable)

- Recommendations do not cause effects automatically.
- Effects are observed only in parallel.
- Trajectories are comparative, not explanatory.
- No recommendation generates decisions inside STRATA.

3.3 Data Flow (Recommendation to Trajectory)

Flow A: Ingestion
- Input: recommendation texts and their production context.
- Output: recommendation snapshots with explicit source references.
- Rule: preserve temporal context and intended action.

Flow B: Assembly
- Input: multiple snapshots ordered by temporal context.
- Output: recommendation trajectory (non-continuous sequence).
- Rule: do not interpolate or smooth across gaps.

3.4 Interfaces (Conceptual, not code)

RecommendationSnapshot
- RecommendationText
- ContextConditions
- IntendedAction
- ExpectedOutcome
- SourceReference
- TemporalContext
- Immutable after creation

Aggregate: RecommendationTrajectory
- Ordered sequence of RecommendationSnapshots
- Trajectory metadata

-------------------------------------------------------------------------------
4. Integration Contracts
-------------------------------------------------------------------------------

Allowed dependencies (unidirectional):
- NarrativeObservationContext
- FourthDimensionSystem
- PatchTrajectoryAnalysis

Forbidden dependencies:
- Core Domain
- Land Use
- Biophysical simulation
- Automated feedback

-------------------------------------------------------------------------------
5. LLM Assistance (Cognitive Support Only)
-------------------------------------------------------------------------------

Allowed uses:
- Extract candidate discursive elements and recommendation snapshots.
- Organize statements into proposed structures.
- Highlight contradictions and absences.

Prohibited uses:
- Creating official system states.
- Validating hypotheses.
- Assigning causality.
- Producing scientific conclusions.

All LLM output is non-authoritative support and must be reviewed.

-------------------------------------------------------------------------------
6. Review Checklist
-------------------------------------------------------------------------------

- Are sources always explicit?
- Is any causal inference sneaking in?
- Are contradictions preserved?
- Are temporal gaps explicit?
- Are outputs framed as observations, not conclusions?
