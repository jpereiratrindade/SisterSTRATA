# ProcessualInferenceContext
## Domain-Driven Design — Versão 0.1

### Contexto
STRATA / IdeaWalker

---

## 1. Propósito do Bounded Context

O ProcessualInferenceContext é responsável por inferir estados e trajetórias
de sistemas socioecológicos a partir de evidências processuais declaradas,
extraídas de documentos científicos e outros artefatos analíticos.

Este contexto NÃO modela diretamente o sistema socioecológico real.
Ele modela a inferência sobre o estado desse sistema, a partir de processos,
relações causais e recorrências observadas na literatura.

---

## 2. Linguagem Ubíqua (Ubiquitous Language)

- Processo
- Declaração de Processo (ProcessStatement)
- Dimensão (Ecológica, Produtiva, Social)
- Relação Causal
- Trajetória
- Sinal de Trajetória
- Inferência de Estado
- Coerência Interdimensional
- Janela Temporal
- Confiança

---

## 3. Bounded Context

Nome: ProcessualInferenceContext

Responsabilidade central:
Inferir estados e trajetórias a partir da agregação incremental
de processos declarados.

Limites explícitos:
- Não realiza simulação biofísica
- Não classifica normativamente sistemas
- Não prevê estados futuros de forma determinística

---

## 4. Aggregate Root

### SystemInferenceAggregate

Representa a inferência acumulada sobre um sistema analisado
em um recorte espaço-temporal definido.

Contém:
- Conjunto de ProcessStatements
- Relações causais inferidas
- Sinais de trajetória
- Diagnósticos de estado derivados

Invariantes:
- Todo estado é derivado de processos
- Nenhuma inferência ocorre sem evidência declarada
- Processos não são apagados, apenas acumulados ou reponderados

---

## 5. Entidades

### ProcessStatement

Representa uma afirmação processual extraída de um documento.

Atributos principais:
- description (string)
- dimension (DimensionTag)
- affectedVariables (lista de strings)
- causalDirection (positive | negative | neutral)
- confidenceLevel
- sourceReference
- temporalContext

Origem típica:
IdeaWalker / Scientific Ingestion

---

### CausalLink

Representa uma relação inferida entre dois ProcessStatements.

Atributos:
- originProcess
- targetProcess
- relationType (pressure, mediation, feedback)
- strength (recorrência relativa)
- consensusLevel

---

### TrajectorySignal

Representa um padrão emergente reconhecido no conjunto de processos.

Exemplos:
- pressure_increasing
- functional_stability
- interdimensional_divergence
- adaptive_stagnation

---

## 6. Value Objects

### DimensionTag
- ecological
- productive
- social

### ConfidenceLevel
- low
- medium
- high
(ou escala contínua 0–1)

### TemporalWindow
- snapshot
- short_term
- long_term
- accumulated

---

## 7. Domain Services

### ProcessAggregationService
- Agrega ProcessStatements
- Calcula recorrência
- Atualiza pesos e confiança

---

### CausalInferenceService
- Detecta cadeias causais dominantes
- Diferencia consenso e controvérsia
- Mantém grafo causal fraco

---

### StateDerivationService
- Deriva diagnósticos processuais
- Nunca classifica de forma normativa
- Produz estados descritivos (ex.: pressão crescente)

---

### TrajectoryAssessmentService
- Compara o conjunto atual com padrões conhecidos
- Calcula proximidade com trajetórias típicas
- Gera sinais de tendência

---

## 8. Integração com Outros Contextos

Entrada:
- NarrativeContext
- DiscursiveContext
- ScientificIngestion (IW)

Saída:
- FourthDimensionContext (Resiliência)
- Visualization / Decision Support

---

## 9. Anti-Corruption Layer (IW → STRATA)

- ProcessStatements entram como strings
- Nenhuma ontologia rígida é imposta na entrada
- Normalização ocorre apenas internamente ao contexto

---

## 10. Status do Documento

Versão: 0.1
Natureza: Fundacional
Uso previsto:
- Referência de implementação
- Guia de desenvolvimento
- Base para evolução do modelo de inferência
