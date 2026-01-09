===============================================================================
DTO — RecommendationSnapshot
===============================================================================

STATUS: proposed (non-canonical; requires human validation)
SCOPE: data transfer only (read-only)
SOURCE OF TRUTH: `DDD_DiscursiveSystem_RecommendationTrajectory_STRATA.md`

-------------------------------------------------------------------------------
1. PURPOSE
-------------------------------------------------------------------------------

This DTO carries a single recommendation snapshot for comparative trajectories
without implying causality or decision authority.

-------------------------------------------------------------------------------
2. FIELDS (ALLOWED)
-------------------------------------------------------------------------------

Identity
- RecommendationSnapshotID

Recommendation Content
- RecommendationText
- ContextConditions
- IntendedAction
- ExpectedOutcome

Provenance
- SourceReference
- TemporalContext

-------------------------------------------------------------------------------
3. FIELDS (EXPLICITLY PROHIBITED)
-------------------------------------------------------------------------------

- Any causal assertion (explicit or implied)
- Normative directives for execution
- Decision outputs
- Simulation parameters
- Land use changes
- Feedback hooks to Core Domain

-------------------------------------------------------------------------------
4. ORIGIN OF DATA
-------------------------------------------------------------------------------

- Technical recommendations, reports, or bulletins
- Each snapshot must keep explicit source linkage

-------------------------------------------------------------------------------
5. ALLOWED USES
-------------------------------------------------------------------------------

- Read-only display in UI
- Comparative ordering in trajectories
- Parallel analysis with spatial or ecological trajectories

-------------------------------------------------------------------------------
6. PROHIBITED USES
-------------------------------------------------------------------------------

- Automated decision-making
- Causal inference or scientific validation
- Use as input to simulation or land use systems
