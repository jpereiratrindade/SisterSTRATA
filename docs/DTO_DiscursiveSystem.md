===============================================================================
DTO — DiscursiveSystem
===============================================================================

STATUS: proposed (non-canonical; requires human validation)
SCOPE: data transfer only (read-only)
SOURCE OF TRUTH: `DDD_DiscursiveSystem_RecommendationTrajectory_STRATA.md`

-------------------------------------------------------------------------------
1. PURPOSE
-------------------------------------------------------------------------------

This DTO carries discursive system observations across contexts without
introducing causal inference, decision logic, or normalization.

-------------------------------------------------------------------------------
2. FIELDS (ALLOWED)
-------------------------------------------------------------------------------

Identity
- DiscursiveSystemID

Discursive Claims
- DeclaredProblems
- DeclaredActions
- AllegedMechanisms
- ExpectedEffects

Provenance
- SourceReferences
- TemporalContext
- InterpretationMetadata

-------------------------------------------------------------------------------
3. FIELDS (EXPLICITLY PROHIBITED)
-------------------------------------------------------------------------------

- Any causal assertion (explicit or implied)
- Normalized or merged truths
- Derived decisions or management directives
- Simulation parameters
- Land use changes
- Feedback hooks to Core Domain

-------------------------------------------------------------------------------
4. ORIGIN OF DATA
-------------------------------------------------------------------------------

- Textual sources: interviews, documents, questionnaires, bulletins
- Each item must keep explicit source linkage

-------------------------------------------------------------------------------
5. ALLOWED USES
-------------------------------------------------------------------------------

- Read-only display in UI
- Comparative analysis with other observational trajectories
- Parallel reading with FourthDimensionSystem

-------------------------------------------------------------------------------
6. PROHIBITED USES
-------------------------------------------------------------------------------

- Automated decision-making
- Causal inference or scientific validation
- Use as input to simulation or land use systems
