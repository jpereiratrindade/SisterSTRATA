===============================================================================
DDD – TRAJECTORY IMPACT PROFILE
STRATA – ANÁLISE DE IMPACTO COMO DEFORMAÇÃO DE TRAJETÓRIA
===============================================================================

VERSÃO: 1.0
STATUS: IMPLEMENTED (V1.0 - C++20)
NATUREZA: Observacional / Read-only
ESCOPO: Análise de Impacto Sistêmico
DEPENDE DE: FourthDimensionSystem, PatchTrajectoryAnalysis
NÃO DEPENDE DE: Uso da Terra, Decisão, Simulação Dinâmica, IA

-------------------------------------------------------------------------------
1. FUNDAMENTO CONCEITUAL
-------------------------------------------------------------------------------

No STRATA, impacto NÃO é tratado como:
- valor escalar
- juízo normativo
- resultado desejável ou indesejável
- medida de sucesso ou fracasso

Impacto é definido como:

DEFORMAÇÃO OBSERVÁVEL DE UMA TRAJETÓRIA
EM RELAÇÃO A UMA REFERÊNCIA EXPLÍCITA.

Uma recomendação, intervenção ou mudança de contexto
não "causa" diretamente um efeito,
mas desloca o sistema dentro do espaço de trajetórias possíveis.

-------------------------------------------------------------------------------
2. DEFINIÇÃO DO BOUNDED CONTEXT
-------------------------------------------------------------------------------

Nome:
TrajectoryImpactProfile

Tipo:
Bounded Context Analítico (Read-only)

Responsabilidade Central:
Descrever como uma trajetória observada
se afasta, se aproxima ou se reorganiza
em relação a uma trajetória ou referência definida.

Este contexto:
- NÃO decide
- NÃO prescreve
- NÃO retroalimenta a simulação
- NÃO classifica impacto como positivo ou negativo

-------------------------------------------------------------------------------
3. PRINCÍPIOS INEGOCIÁVEIS
-------------------------------------------------------------------------------

1) Impacto é sempre RELATIVO a uma referência explícita.
2) Impacto é descrito como PADRÃO DE DESVIO, não como valor absoluto.
3) Nenhuma inferência causal é assumida.
4) Nenhuma decisão é embutida no modelo.
5) Nenhuma métrica isolada define impacto.
6) Toda leitura é contextual e reversível.

Violação de qualquer princípio caracteriza erro conceitual.

-------------------------------------------------------------------------------
4. LINGUAGEM UBÍQUA (CONCEITOS-CHAVE)
-------------------------------------------------------------------------------

Trajetória Observada:
Sequência efetivamente registrada de TimeSlices.

Trajetória de Referência:
Sequência utilizada como base de comparação.
Pode ser:
- VegetationSystemOriginal (potencial)
- trajetória histórica
- cenário controle explícito

Deformação de Trajetória:
Diferença estrutural entre duas trajetórias,
expressa como mudança de padrões espaciais e temporais.

Envelope de Possibilidades:
Conjunto de trajetórias plausíveis dado um sistema de referência.

-------------------------------------------------------------------------------
5. ENTIDADE PRINCIPAL
-------------------------------------------------------------------------------

Entity: TrajectoryImpactProfile
Identidade: ImpactProfileID

Descrição:
Representa o conjunto de descrições analíticas
sobre como uma trajetória se deformou
em relação a uma referência explícita.

A entidade é IMUTÁVEL após criada.

-------------------------------------------------------------------------------
6. ATRIBUTOS DO TrajectoryImpactProfile
-------------------------------------------------------------------------------

- ObservedTrajectoryID
- ReferenceTrajectoryID

- StructuralDeviationSummary
  (descrição sintética das deformações observadas)

- TemporalBehaviorProfile
  (persistência, volatilidade, pulsos, rupturas)

- SpatialOrganizationShift
  (fragmentação, coalescência, reorganização)

- ConnectivityDeviation
  (manutenção, perda ou reorganização funcional)

- Metadata
  - critérios utilizados
  - escalas consideradas
  - limitações explícitas

-------------------------------------------------------------------------------
7. VALUE OBJECTS
-------------------------------------------------------------------------------

Value Object: StructuralDeviation
Representa padrões como:
- aumento de volatilidade estrutural
- perda de persistência espacial
- reorganização sem recomposição funcional

---

Value Object: TemporalDeviationPattern
Representa:
- trajetórias estáveis
- trajetórias pulsáteis
- trajetórias erosivas
- trajetórias reorganizativas

---

Value Object: ReferenceFrame
Define explicitamente:
- qual é a referência
- por que foi escolhida
- quais limites possui

-------------------------------------------------------------------------------
8. AGREGADO
-------------------------------------------------------------------------------

Aggregate Root:
TrajectoryImpactProfile

Contém:
- referência explícita
- trajetória observada
- conjunto de descrições de deformação

Invariantes:
- nenhuma alteração nos dados de origem
- nenhuma métrica recalculada
- nenhuma decisão embutida

-------------------------------------------------------------------------------
9. DOMAIN SERVICES
-------------------------------------------------------------------------------

Service: TrajectoryImpactAnalyzer

Responsabilidades:
- receber duas trajetórias (observada e referência)
- consumir métricas de PatchTrajectory
- identificar padrões de desvio estrutural
- produzir um TrajectoryImpactProfile

O serviço:
- NÃO cria trajetórias
- NÃO modifica TimeSlices
- NÃO normaliza impacto

-------------------------------------------------------------------------------
10. RELAÇÃO COM OUTROS CONTEXTOS
-------------------------------------------------------------------------------

Depende de:
- FourthDimensionSystem
- PatchTrajectoryAnalysis
- VegetationSystemOriginal (quando usado como referência)

Não depende de:
- Sistemas de decisão
- Uso da terra ativo
- IA
- Interface gráfica

Pode ser consumido por:
- UI científica
- Módulos narrativos
- Assistência cognitiva (LLM), apenas como texto

-------------------------------------------------------------------------------
11. INTEGRAÇÃO COM ASSISTÊNCIA COGNITIVA
-------------------------------------------------------------------------------

A IA recebe APENAS:
- descrições consolidadas
- padrões identificados
- metadados de referência

A IA:
- NÃO calcula impacto
- NÃO compara métricas brutas
- NÃO recomenda ações

Seu papel é exclusivamente interpretativo e narrativo.

-------------------------------------------------------------------------------
12. DIRETRIZES DE IMPLEMENTAÇÃO
-------------------------------------------------------------------------------

1) Implementar como estrutura analítica imutável.
2) Priorizar clareza semântica sobre compressão numérica.
3) Exigir referência explícita em toda análise.
4) Registrar limitações como parte do resultado.
5) Nunca expor este contexto como mecanismo decisório.

-------------------------------------------------------------------------------
13. NOTA FINAL À EQUIPE
-------------------------------------------------------------------------------

TrajectoryImpactProfile NÃO existe para dizer
"o que deveria ser feito".

Ele existe para tornar legível
como o sistema PASSOU A SER
após determinadas condições terem sido observadas.

Impacto não governa o sistema.
Impacto torna a trajetória pensável.

===============================================================================
FIM DO DOCUMENTO
===============================================================================
