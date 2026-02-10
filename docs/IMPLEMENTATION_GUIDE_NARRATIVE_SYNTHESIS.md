# Implementation Guide Narrative Synthesis

Status: proposed, actionable
Scope: IW -> STRATA ingestion synthesis and narrative graph representation
Reference epistemic base: `docs/NOTE_EPISTEMOLOGIA_INGEST_IW_STRATA.md`

## 1. Objective

Implement ingestion synthesis and narrative proximity visualization in STRATA without changing Core Domain semantics and without introducing causal inference.

## 2. Non-Negotiable Principles

- All outputs are observational and read-only.
- No generated artifact can mutate simulation state or trajectory state.
- Narrative proximity is epistemic, not causal.
- Resilience is not computed in these artifacts.
- Graphs are decision-support for interpretation, not model execution inputs.

## 3. Deliverables

### 3.1 Canonical Synthesis Report

- Trigger: at the end of IW ingest flows.
- Files:
  - `<projectRoot>/reports/ingestion/IngestionSynthesisReport.latest.json`
  - `<projectRoot>/reports/ingestion/IngestionSynthesisReport.latest.md`
  - timestamped copies for both formats.
- JSON is the source of truth.
- Markdown is a deterministic human-readable derivative.

Mandatory JSON blocks:
- `schemaVersion`, `reportType`, `generatedAt`, `trigger`, `sourcePath`
- `epistemicStatus`
- `summary`
- `contexts`
- `skipReasons`
- `artifacts`
- `narrativeContextGraph`

Mandatory epistemic status values:
- `type = observational_synthesis`
- `allowsResilienceInference = false`
- `requiresSpatialTemporalData = true`

### 3.2 Narrative Context Graph

- Type: derived analytical view.
- Nodes: narrated contexts.
- Edges: narrative similarity only.
- Distance: `1 - similarity`.
- Current distance method: `epistemic_narrative_jaccard_v1`.
- Traceability in node payload:
  - `observationIds`
  - `artifactIds`
  - `topTokens`

### 3.3 UI Graph Controls

- `Min Similarity` threshold.
- `Top K links per node` sparsification.
- `Hide Isolated`.
- `Focus Selected Node` (one-hop neighborhood).
- Labels default off, enabled for hover/selected or explicit "Show Labels".
- Visible color legend for node dimensions (`ecological`, `productive`, `social`, `mixed`).

### 3.4 SGS UX Robustness

- `Strategic Global Synthesis` must not fail silently.
- Before requesting LLM:
  - validate that at least one observational layer has records.
  - validate that LLM service is configured.
- During execution:
  - show explicit pending status.
- After execution:
  - show success/failure status and persist interpretation snapshot when requested by user.

## 4. Architecture Boundary

- Placement: Application + UI layers.
- Forbidden: Core aggregates, simulation kernel, world model dynamics.
- Any future service must remain a derived read model fed by persisted observational contexts.

## 5. Validation Criteria

- Report files are generated for ingest flows.
- JSON and Markdown are coherent for the same run.
- Graph is renderable from persisted narrative contexts.
- Selected node can be traced back to ingest records via IDs.
- No simulation output changes when enabling/disabling graph features.

## 6. Team Task Breakdown

1. Ingestion artifact generation and tests.
2. Narrative graph generation and traceability fields.
3. UI rendering with sparsification/focus controls.
4. Documentation update in user and technical manuals.
5. Regression suite and acceptance checks.

## 7. Out of Scope

- Causal modeling.
- Automatic resilience scoring.
- Policy recommendation ranking.
- Feeding graph metrics into trajectory simulation.
