# Narrative Observation Context: Contracts & Flows

STATUS: living document (non-canonical; requires human validation)
SCOPE: observational / declarative / read-only context
SOURCE OF TRUTH: `DDD_NarrativeObservationContext.md`

-------------------------------------------------------------------------------
1. Purpose and Epistemic Contract
-------------------------------------------------------------------------------

This context exists to register narrative observations about territory without
turning them into causal explanations or management decisions.

Architectural decision (explicit): no causal inference and no semantic
normalization. This is a deliberate choice, not a technical limitation.

-------------------------------------------------------------------------------
2. Core Objects (Ubiquitous Language)
-------------------------------------------------------------------------------

Narrative Source:
- Interview, technical document, report, minutes, historical record.

Semantic State:
- Declarative configuration derived from a narrative source, bound to time and
  production context.

Narrative Axis:
- Recurrent discourse dimension (e.g., management, abandonment, intensification,
  conservation, conflict).

Narrative Trajectory:
- Ordered sequence of semantic states across time.
- Does not assume continuity, periodicity, or temporal completeness.

-------------------------------------------------------------------------------
3. Invariants (Non-Negotiable)
-------------------------------------------------------------------------------

- No narrative is treated as factual truth.
- No causal inference is permitted.
- No management, land use, or simulation decisions derive from this context.
- Every interpretation must keep explicit linkage to its source.
- Semantic states are observations, not explanations.
- Ambiguity and contradiction are valid states.
- No retroactive modification of existing states.
- No forced normalization across narratives.

-------------------------------------------------------------------------------
4. Data Flow (Text to State)
-------------------------------------------------------------------------------

Flow A: Ingestion
- Input: narrative source (document, interview, report).
- Output: candidate semantic states with explicit source reference.
- Rule: preserve the source, timestamp, and production context.

Flow B: Versioning
- Input: new or updated sources (revisions, new interviews).
- Output: new semantic states; never overwrite previous ones.
- Rule: maintain coexistence of contradictory states.

Flow C: Trajectory Assembly
- Input: multiple semantic states, ordered by temporal context.
- Output: narrative trajectory (non-continuous sequence).
- Rule: do not interpolate or smooth across gaps.

-------------------------------------------------------------------------------
5. LLM Assistance (Cognitive Support Only)
-------------------------------------------------------------------------------

Allowed uses:
- Identify possible narrative axes.
- Organize excerpts into candidate semantic states.
- Suggest interpretive syntheses.
- Highlight internal contradictions.

Prohibited uses:
- Creating official system states.
- Validating hypotheses.
- Assigning causality.
- Producing scientific conclusions.

All LLM output is non-authoritative support and must be reviewed.

-------------------------------------------------------------------------------
6. Integration Contracts
-------------------------------------------------------------------------------

Allowed dependencies (unidirectional):
- FourthDimensionSystem (for temporal comparison only).
- Visualization and analysis systems.

Forbidden dependencies:
- Core Domain.
- Land Use or decision systems.
- Biophysical simulation or automated feedback.

-------------------------------------------------------------------------------
7. Interfaces (Conceptual, not code)
-------------------------------------------------------------------------------

NarrativeState
- SemanticAxes
- SourceReference
- TemporalContext
- SpatialScope (optional)
- InterpretationMetadata
- Immutable after creation

Aggregate: NarrativeObservationSystem
- Versioned collection of NarrativeStates
- Corpus-level metadata

Value Objects
- SourceReference (type, identifier, production date, authorship)
- SemanticAxis (label, description, abstraction level)
- TemporalContext (relative ordering only)

-------------------------------------------------------------------------------
8. Review Checklist
-------------------------------------------------------------------------------

- Is every state linked to its source?
- Is there any causal or normative statement sneaking in?
- Are contradictions preserved rather than resolved?
- Are temporal gaps explicitly maintained?
- Is any output positioned as scientific truth? (must be no)

