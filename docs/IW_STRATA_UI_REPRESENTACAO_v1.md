# IW -> STRATA: Requisitos de Representacao UI (v1)

STATUS: proposto (nao canonico; requer validacao humana)  
SCOPE: representacao de dados e ingestao em Application/UI (sem mudanca de Core Domain)  
SOURCE OF TRUTH:
- `docs/NARRATIVE_OBSERVATION_CONTEXT_CONTRACTS.md`
- `docs/DISCURSIVE_SYSTEM_RECOMMENDATION_TRAJECTORY_CONTRACTS.md`
- `docs/DTO_DiscursiveSystem.md`
- `docs/DTO_RecommendationSnapshot.md`
- `docs/DTO_RecommendationTrajectory.md`

---

## 1. Objetivo

Definir um contrato unico para:
- mapeamento de artefatos IW para campos de UI e DTOs do STRATA
- precedencia de ingestao por bundle
- namespace de metadados
- criterio de promocao de metadata para campo estruturado
- metricas de cobertura de ingestao

Este documento nao altera modelo cientifico e nao introduz inferencia causal.

---

## 2. Artefatos IW observados (dataset Embrapa_Volk)

Origem analisada: `/run/media/jpereiratrindade/labeco10T/Teste/Embrapa_Volk`

Arquivos recorrentes por bundle:
- `IWBundle.json`
- `DiscursiveSystem.json`
- `NarrativeObservation.json`
- `TrajectoryAnalogies.json`
- `AllegedMechanisms.json`
- `InterpretationLayers.json`
- `DiscursiveContext.json`
- `BaselineAssumptions.json`
- `TemporalWindowReference.json`
- `SourceProfile.json`
- `Manifest.json`
- `EpistemicValidationReport.json`

Exemplo real de bundle:
- `/run/media/jpereiratrindade/labeco10T/Teste/Embrapa_Volk/strata/consumables/20260210_074343_marques2017the-effects-of-phosphoro.pdf/`

---

## 3. Precedencia de ingestao

Objetivo: evitar duplicidade e preservar granularidade canonica do bundle.

Regra de precedencia por `artifactId`:
1. `DiscursiveSystem.json`, `NarrativeObservation.json`, `TrajectoryAnalogies.json`
2. `IWBundle.json`
3. Artefatos satelite (`AllegedMechanisms.json`, `InterpretationLayers.json`, `DiscursiveContext.json`, `BaselineAssumptions.json`, `TemporalWindowReference.json`, `SourceProfile.json`)

Regras:
- Se arquivo de nivel superior existir, ele e fonte primaria daquele contexto.
- `IWBundle.json` so complementa contexto ausente no nivel superior.
- Artefatos satelite so complementam campos faltantes.
- `Manifest.json` e `source.artifactId` sao usados para correlacao.

---

## 4. Correlacao e IDs

Chave de correlacao:
- `artifactId` (ou fallback para `filename` quando `artifactId` ausente)

Regras de ID em DTO:
- Discursive: `DS-IW-<artifactId>-<idx>`
- Narrative: `OBS-IW-<artifactId>-<idx>`
- Recommendation: `REC-IW-<artifactId>-<idx>`

Objetivo:
- evitar colisoes entre ingestoes
- manter rastreabilidade entre contextos

---

## 5. Namespace de metadados

Separacao obrigatoria:
- `iw.*` para campos importados do IW
- `strata.*` para campos internos/derivados no STRATA

Exemplos:
- `iw.evidenceSnippet`
- `iw.sourceSection`
- `iw.pageRange`
- `iw.confidence`
- `iw.contextuality`
- `iw.baselineAssumptions`
- `iw.temporalWindowReferences`
- `iw.sourceProfile`
- `iw.interpretationLayers`
- `iw.discursiveContext`

---

## 6. Mapeamento IW -> UI -> DTO

### 6.1 Discursive Context

Fonte primaria:
- `DiscursiveSystem.json`
- fallback: `IWBundle.json`

Complementos:
- `AllegedMechanisms.json`
- `InterpretationLayers.json`
- `DiscursiveContext.json`
- `BaselineAssumptions.json`
- `TemporalWindowReference.json`
- `SourceProfile.json`

Mapeamento base:
- `declaredProblems` -> form Discursive -> `DiscursiveSystemDTO.declaredProblems`
- `declaredActions` -> form Discursive -> `DiscursiveSystemDTO.declaredActions`
- `allegedMechanisms` -> form Discursive -> `DiscursiveSystemDTO.allegedMechanisms`
- `expectedEffects` -> form Discursive -> `DiscursiveSystemDTO.expectedEffects`
- `sourceReferences` -> form Discursive -> `DiscursiveSystemDTO.sourceReferences`
- `temporalContext` -> form Discursive -> `DiscursiveSystemDTO.temporalContext`

Mapeamento complementar (metadata):
- `baselineAssumptions` -> `DiscursiveSystemDTO.interpretationMetadata["iw.baselineAssumptions"]`
- `discursiveContext` -> `DiscursiveSystemDTO.interpretationMetadata["iw.discursiveContext"]`
- `interpretationLayers` -> `DiscursiveSystemDTO.interpretationMetadata["iw.interpretationLayers"]`
- `temporalWindowReferences` -> `DiscursiveSystemDTO.interpretationMetadata["iw.temporalWindowReferences"]`
- `sourceProfile` -> `DiscursiveSystemDTO.interpretationMetadata["iw.sourceProfile"]`

### 6.2 Narrative Context

Fonte primaria:
- `NarrativeObservation.json`
- fallback: `IWBundle.json` (`narrativeObservations`)

Mapeamento base:
- `source` -> form Narrative -> `NarrativeStateDTO.source`
- `temporalContext` -> form Narrative -> `NarrativeStateDTO.temporalContext`
- `intent` -> form Narrative -> `NarrativeStateDTO.intent`
- `axes` -> form Narrative -> `NarrativeStateDTO.axes`
- `spatialScope` -> form Narrative -> `NarrativeStateDTO.spatialScope`

Mapeamento complementar (metadata):
- `observation`, `evidenceSnippet`, `sourceSection`, `pageRange`, `confidence`, `context`, `contextuality`, `limits`, `evidence`
-> `NarrativeStateDTO.metadata["iw.<campo>"]`

### 6.3 Recommendation Trajectory

Fonte primaria:
- `TrajectoryAnalogies.json`
- fallback: `IWBundle.json` (`trajectoryAnalogies`)

Mapeamento:
- `analogy` -> `RecommendationSnapshotDTO.recommendationText`
- `justification` -> `RecommendationSnapshotDTO.expectedOutcome`
- `scope` -> `RecommendationSnapshotDTO.contextConditions`
- `source.artifactId` -> `RecommendationSnapshotDTO.sourceReference.sourceId`

Regra:
- nao truncar em um item: cada analogia gera um snapshot.

---

## 7. Requisitos de UI (v1)

Sem alterar Core Domain:

Narrative Form:
- Expor editor de metadata (`key/value`) com suporte a chaves `iw.*`
- Exibir metadados importados no detalhe do registro

Discursive Form:
- Manter metadata livre
- Adicionar blocos colapsaveis para visualizacao/edicao de:
  - `iw.baselineAssumptions`
  - `iw.discursiveContext`
  - `iw.interpretationLayers`
  - `iw.temporalWindowReferences`
  - `iw.sourceProfile`

Recommendation Form:
- manter estrutura atual
- garantir que multiplas analogias sejam visiveis como snapshots distintos

Escopo de analise LLM por contexto:
- Discursive, Narrative e Recommendation devem permitir selecao de 1..N registros para analise com Qwen
- quando `usar selecionados` estiver ativo:
  - se nenhum registro estiver selecionado, exibir erro explicito de selecao vazia
  - nao executar analise com contexto vazio
- quando `usar selecionados` estiver inativo:
  - executar com todos os registros visiveis daquele contexto

Memoria epistemica LLM:
- permitir selecao de snapshots de interpretacao
- permitir exportacao em Markdown:
  - `Export Selected .md`
  - `Export Visible .md`
- manter exportacao contextual (filtrada por `intent` da aba atual)

Context Graph (Narrative):
- exibir grafo epistemico de contexto narrativo em aba dedicada
- no:
  - tamanho proporcional a quantidade de observacoes narrativas
  - cor por dimensao dominante (`ecological`, `productive`, `social`, `mixed`)
- aresta:
  - peso/espessura proporcional a similaridade (Jaccard)
  - distancia derivada como `1 - similarity` (sem interpretacao causal)
- controles minimos:
  - filtro de similaridade minima
  - limite de conectividade por no (`Top-K por no`)
  - opcao para ocultar nos isolados
  - modo foco por no selecionado (1 salto)
  - legenda visual de cores por dimensao epistemica
  - tooltip com `observationIds`, `artifactIds` e `topTokens`

---

## 8. Criterio de promocao de campo

Um metadata `iw.*` vira campo estruturado de formulario somente se:
1. recorrencia >= 30% dos bundles validos
2. impacto analitico confirmado por revisao humana
3. sem violar contratos epistemologicos dos contextos

Enquanto nao promovido:
- permanece em metadata com chave estavel

---

## 9. Metricas de cobertura de ingestao

Por execucao de ingestao:
- total de bundles detectados
- total de bundles ingeridos
- total de artefatos por tipo
- campos mapeados por contexto (discursive/narrative/recommendation)
- campos descartados (lista com motivo)
- cobertura (%) = campos mapeados / campos reconhecidos

Saida minima esperada em log:
- `[IW Ingest] bundle=<artifactId> context=<...> mapped=<n> skipped=<n>`
- `[IW Ingest Summary] bundles=<n> discursive=<n> narrative=<n> recommendation=<n> coverage=<x%>`

Artefato canônico de sintese (automatico ao fim do ingest):
- JSON (fonte de verdade): `<projectRoot>/reports/ingestion/IngestionSynthesisReport.latest.json`
- Markdown (derivado humano): `<projectRoot>/reports/ingestion/IngestionSynthesisReport.latest.md`
- versoes com timestamp:
  - `IngestionSynthesisReport_<YYYYMMDD_HHMMSS>.json`
  - `IngestionSynthesisReport_<YYYYMMDD_HHMMSS>.md`
- status epistemologico fixo:
  - `type = observational_synthesis`
  - `allowsResilienceInference = false`
  - `requiresSpatialTemporalData = true`
- incluir `narrativeContextGraph` no JSON com:
  - `distanceType = epistemic_narrative_jaccard_v1`
  - `causalInterpretationAllowed = false`
  - `nodes[]` com `observationIds` e `artifactIds` para rastreabilidade

---

## 10. Fora de escopo (v1)

- alterar aggregates do Core Domain
- introduzir inferencia causal
- transformar metadata em verdade cientifica
- aplicar recommendation automaticamente em simulacao

---

## 11. Implementacao aplicada (estado atual)

Aplicado em `Session`, `IWMapper`, `NarrativePanel`, `DiscursiveSystemPanel` e testes de mapper.

Ingestao IW:
- ingestao orientada a bundle com precedencia por contexto
- correlacao por `artifactId` (fallback controlado) e IDs deterministicas por contexto
- suporte a multiplos snapshots de recommendation (sem truncar para o primeiro item)
- merge de artefatos satelite em `iw.*` para contexto discursive
- upsert para evitar duplicidade em reingestao
- logs por contexto e resumo de cobertura

Higiene de ingestao:
- quando bundles canonicos sao detectados, JSON standalone e ignorado para evitar duplicidade
- filtros para aceitar apenas payloads IW reconhecidos
- logs por contexto sao emitidos somente quando `mapped > 0` ou `skipped > 0`

UI:
- Narrative: editor de metadata (`key/value`) + campos de evidencia (`iw.evidenceSnippet`, `iw.sourceSection`, `iw.pageRange`)
- Narrative: coluna de metadata na grade com tooltip de detalhe
- Narrative: selecao de observacoes para analise Qwen (`usar selecionados`)
- Narrative: aba `Context Graph` com visualizacao de proximidade/distancia epistemica entre contextos
- Discursive: blocos colapsaveis para `iw.baselineAssumptions`, `iw.discursiveContext`, `iw.interpretationLayers`, `iw.temporalWindowReferences`, `iw.sourceProfile`
- Discursive: selecao de sistemas para analise Qwen (`usar selecionados`)
- Recommendation: selecao de snapshots para analise Qwen (`usar selecionados`)
- Discursive e Narrative: campos ajustados para largura disponivel da janela (responsividade em formularios e tabelas)
- Memoria epistemica (componente compartilhado): selecao de snapshots + exportacao `.md` (selecionados/visiveis)

Escopo estrategico:
- Global Synthesis permanece holistico (analisa o contexto completo), sem selecao parcial de registros
- Global Synthesis deve exibir feedback explicito de execucao:
  - erro de ausencia de dados no projeto
  - erro de indisponibilidade do servico LLM
  - status de auditoria em andamento / concluida

Testes:
- casos adicionais em `tests/application/IWMapperTest.cpp` cobrindo:
  - multiplas analogias
  - parse de `history` com namespace `iw.*`
  - parse de `systems` em Discursive

---

## 12. Pendencias de validacao

- validacao humana da UX dos campos longos em diferentes resolucoes de tela
- eventual limpeza de dados historicos ja persistidos antes dos filtros de ingestao
- decisao de promocao de metadados recorrentes para campos estruturados (criterio da secao 8)
- avaliar se Global Synthesis deve manter apenas modo holistico ou receber modo por subconjunto

---

## 13. Operacao (LLM com selecao de registros)

Discursive:
- usar a coluna `Sel` na grade para marcar 1..N sistemas
- manter `Use selected records only` ativo para limitar o escopo da analise
- acoes suportadas: `Ask Qwen to Propose System` e `Evaluate Logical Coherence`

Narrative:
- usar a coluna `Sel` na grade para marcar 1..N observacoes
- manter `Use selected observations only` ativo para limitar o escopo da analise
- acao suportada: `Analyze Themes with Qwen`

Recommendation:
- usar a coluna `Sel` na grade para marcar 1..N snapshots
- manter `Use selected snapshots only` ativo para limitar o escopo da analise
- acao suportada: `Analyze Trajectory with Qwen`

Memoria Epistemica:
- selecionar snapshots no historico da aba ativa
- exportar com:
  - `Export Selected .md`
  - `Export Visible .md`
- a exportacao e filtrada pelo `intent` da aba e preserva rastreabilidade (`snapshotId`, `createdAt`, `sourceBundleId`, `promptVersion`)
