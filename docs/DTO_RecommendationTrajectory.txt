===============================================================================
DTO — RecommendationTrajectory
===============================================================================

STATUS: proposed (non-canonical; requires human validation)
SCOPE: data transfer only (read-only)
SOURCE OF TRUTH: `DDD_DiscursiveSystem_RecommendationTrajectory_STRATA.md`

-------------------------------------------------------------------------------
1. PURPOSE
-------------------------------------------------------------------------------

This DTO carries an ordered sequence of recommendation snapshots for
comparative analysis only. It does not explain or enforce outcomes.

-------------------------------------------------------------------------------
2. FIELDS (ALLOWED)
-------------------------------------------------------------------------------

Identity
- RecommendationTrajectoryID

Trajectory Content
- RecommendationSnapshots (ordered)
- TrajectoryMetadata

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

- Ordered collection of RecommendationSnapshots
- Each snapshot must keep explicit source linkage

-------------------------------------------------------------------------------
5. ALLOWED USES
-------------------------------------------------------------------------------

- Read-only display in UI
- Comparative analysis across time
- Parallel reading with ecological or spatial trajectories

-------------------------------------------------------------------------------
6. PROHIBITED USES
-------------------------------------------------------------------------------

- Automated decision-making
- Causal inference or scientific validation
- Use as input to simulation or land use systems
