# ProcessualInferenceContext
## Domain-Driven Design (DDD)
### Versão 0.2 — Documento Canônico

Projeto: STRATA / IdeaWalker  
Autor: José Pedro Pereira Trindade  
Status: Estável (Fundacional)

---

## 1. Visão Geral do Domínio

O ProcessualInferenceContext é um Bounded Context do STRATA responsável
pela inferência de estados e trajetórias de sistemas socioecológicos
a partir da agregação incremental de processos declarados.

Este contexto NÃO modela o sistema socioecológico real.
Ele modela a inferência sobre o estado desse sistema,
derivada de evidências processuais extraídas da literatura científica
e de outros artefatos analíticos.

---

## 2. Princípios de Domínio (Invariantes Epistemológicas)

1. Todo estado é derivado de processos.
2. Nenhuma inferência ocorre sem evidência declarada.
3. Estados não são classificados normativamente.
4. Trajetórias são inferências temporais, não atributos fixos.
5. Processos não são apagados; apenas acumulados ou reponderados.
6. Ontologias rígidas não são impostas na entrada do sistema.

Esses princípios derivam diretamente do Modelo Conceitual
(documentado em LaTeX) e são invariantes do domínio.

---

## 3. Linguagem Ubíqua (Ubiquitous Language)

Termos centrais compartilhados por domínio e implementação:

- Processo
- Declaração de Processo (ProcessStatement)
- Dimensão (Ecológica, Produtiva, Social)
- Relação Causal
- Cadeia Causal
- Janela Temporal
- Trajetória
- Sinal de Trajetória
- Inferência de Estado
- Coerência Interdimensional
- Confiança
- Evidência

---

## 4. Bounded Context

Nome: ProcessualInferenceContext

Responsabilidade:
Inferir estados e trajetórias a partir de processos declarados,
respeitando a separação entre extração (IW) e integração (STRATA).

Fora do escopo explícito:
- Simulação biofísica
- Modelos preditivos determinísticos
- Decisão normativa ou prescritiva
- Representação direta do sistema real

---

## 5. Aggregate Root

### SystemInferenceAggregate

Representa a inferência acumulada sobre um sistema analisado
em um recorte espaço-temporal específico.

Responsabilidades:
- Agregar ProcessStatements
- Manter relações causais inferidas
- Produzir diagnósticos de estado derivados
- Sustentar inferências de trajetória

Invariantes:
- Nenhum estado existe sem processos subjacentes
- Inferência sempre rastreável à evidência
- Trajetórias dependem explicitamente de tempo

---

## 6. Entidades

### 6.1 ProcessStatement

Representa uma afirmação processual extraída de um documento.

Atributos:
- description : string
- dimension : DimensionTag
- affectedVariables : list<string>
- causalDirection : positive | negative | neutral
- confidenceLevel : ConfidenceLevel
- sourceReference : string

Atributos temporais (obrigatórios):
- evidenceTimestamp
  (quando o processo ocorreu ou foi observado)
- ingestionTimestamp
  (quando foi ingerido pelo IW)

Observação:
O tempo da evidência é essencial para inferência de trajetória.

---

### 6.2 CausalLink

Representa uma relação causal inferida entre dois ProcessStatements.

Atributos:
- originProcess : ProcessStatement
- targetProcess : ProcessStatement
- relationType : pressure | mediation | feedback
- strength : float (recorrência relativa)
- consensusLevel : ConfidenceLevel

Nota:
CausalLinks são sempre binários.
Cadeias causais longas são inferidas dinamicamente.

---

### 6.3 TrajectorySignal

Representa um padrão emergente inferido a partir da análise temporal.

Atributos:
- signalType
  (ex.: pressure_increasing, functional_stability,
        interdimensional_divergence, adaptive_stagnation)
- temporalWindow : TemporalWindow
- supportingProcesses : list<ProcessStatement>
- confidenceLevel : ConfidenceLevel

Regra:
Um TrajectorySignal só existe associado a uma Janela Temporal.

---

## 7. Value Objects

### 7.1 DimensionTag
- ecological
- productive
- social

---

### 7.2 ConfidenceLevel
- low
- medium
- high
(opcionalmente contínuo: 0–1)

---

### 7.3 TemporalWindow
- snapshot
- short_term
- long_term
- accumulated

Define como o tempo é considerado na inferência.

---

## 8. Domain Services

### 8.1 ProcessAggregationService

Responsabilidade:
- Agregar ProcessStatements
- Calcular recorrência de processos
- Atualizar pesos e níveis de confiança
- Preservar histórico de evidências

---

### 8.2 CausalInferenceService

Responsabilidade:
Inferir relações causais simples e cadeias causais compostas.

Funções:
- Construir grafo causal direcionado
- Inferir cadeias causais de múltiplas ordens
- Detectar mediações interdimensionais
  (ex.: social → produtivo → ecológico)
- Avaliar consenso e controvérsia científica

Observação:
Cadeias causais não são armazenadas como entidades,
mas inferidas a partir do grafo.

---

### 8.3 StateDerivationService

Responsabilidade:
Derivar diagnósticos processuais de estado.

Características:
- Não classifica normativamente
- Não gera rótulos fixos
- Produz descrições como:
  - pressão crescente
  - estabilidade funcional
  - instabilidade produtiva
  - baixa coerência interdimensional

---

### 8.4 TrajectoryAssessmentService

Responsabilidade:
Inferir sinais de trajetória a partir da análise temporal.

Funções:
- Aplicar janelas temporais
- Comparar padrões ao longo do tempo
- Detectar estabilidade, intensificação ou divergência
- Gerar TrajectorySignals

Princípio:
Trajetória é inferência temporal, não atributo do sistema.

---

## 9. Anti-Corruption Layer (IW → STRATA)

Função:
Proteger o domínio do STRATA da variabilidade externa.

Regras:
- ProcessStatements entram como strings
- Nenhuma ontologia rígida é imposta na entrada
- Normalização ocorre apenas internamente
- IW permanece declarativo

---

## 10. Integração com Outros Contextos

Entrada:
- ScientificIngestionContext (IW)
- NarrativeContext
- DiscursiveContext

Saída:
- FourthDimensionContext (Resiliência)
- VisualizationContext
- DecisionSupportContext

---

## 11. Relação com a Quarta Dimensão (Resiliência)

O ProcessualInferenceContext fornece:
- estados derivados
- sinais de trajetória
- coerência interdimensional

A Resiliência emerge como propriedade sistêmica
derivada da interação entre processos,
não como atributo direto do sistema.

---

## 12. Status do Documento

Versão: 0.2  
Natureza: Fundacional  
Uso:
- Documento canônico do domínio
- Referência para implementação
- Base para evolução incremental do STRATA

Próxima revisão prevista:
- Inclusão de exemplos JSON reais
- Testes com dados do IW
