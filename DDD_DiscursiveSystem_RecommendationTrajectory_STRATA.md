
===============================================================================
DDD – DISCURSIVE SYSTEM & RECOMMENDATION TRAJECTORY CONTEXTS
STRATA – SISTEMAS DISCURSIVOS, RECOMENDAÇÕES E TRAJETÓRIAS
===============================================================================

VERSÃO: 1.0
STATUS: PROPOSTA ARQUITETURAL (EM IMPLEMENTAÇÃO CONCEITUAL)
NATUREZA: Observacional / Declarativa / Analítica
ESCOPO: Contextos Auxiliares (NÃO Core Domain)

-------------------------------------------------------------------------------
1. MOTIVAÇÃO GERAL
-------------------------------------------------------------------------------

O STRATA evoluiu mantendo separação rigorosa entre estados biofísicos,
trajetórias espaciais e narrativas observadas.

Este documento introduz dois contextos auxiliares destinados a tornar
explícitos os sistemas de pensamento, prática e recomendação que orbitam
o território, sem convertê-los em causalidade biofísica.

A separação ontológica entre território, discurso, recomendação e efeito
observado é mantida como princípio científico.

===============================================================================
PARTE I — DISCURSIVE SYSTEM CONTEXT (B)
===============================================================================

-------------------------------------------------------------------------------
2. DEFINIÇÃO DO BOUNDED CONTEXT
-------------------------------------------------------------------------------

Nome:
DiscursiveSystemContext

Tipo:
Bounded Context Observacional e Declarativo

Responsabilidade Central:
Reconstruir sistemas discursivos declarados a partir de textos
(entrevistas, documentos, questionários, boletins técnicos),
explicitando problemas, ações, mecanismos alegados e efeitos esperados,
sempre como estrutura discursiva.

-------------------------------------------------------------------------------
3. PRINCÍPIOS INEGOCIÁVEIS
-------------------------------------------------------------------------------

1) Sistemas discursivos NÃO são sistemas ecológicos.
2) Nenhuma causalidade factual é assumida.
3) Todo sistema mantém vínculo explícito com sua fonte.
4) Contradições internas são estados válidos.
5) Ausências discursivas são informação.
6) Nenhum elemento deste contexto altera o Core Domain.

-------------------------------------------------------------------------------
4. CONCEITO CENTRAL
-------------------------------------------------------------------------------

Sistema Discursivo:
Configuração declarada de relações entre problemas, ações, mecanismos
e efeitos, conforme expressos em fontes textuais.

-------------------------------------------------------------------------------
5. ENTIDADES E VALUE OBJECTS
-------------------------------------------------------------------------------

Entity: DiscursiveSystem
Identidade: DiscursiveSystemID

Atributos:
- DeclaredProblems
- DeclaredActions
- AllegedMechanisms
- ExpectedEffects
- SourceReferences
- TemporalContext
- InterpretationMetadata

A entidade é IMUTÁVEL após criada.

Value Objects:
- DeclaredProblem
- DeclaredAction
- AllegedMechanism
- ExpectedEffect
- SourceReference

-------------------------------------------------------------------------------
6. AGREGADO
-------------------------------------------------------------------------------

Aggregate Root:
DiscursiveSystemRepository

Invariantes:
- Nenhum sistema substitui outro
- Múltiplos sistemas podem coexistir para o mesmo território

-------------------------------------------------------------------------------
7. INTEGRAÇÃO COM ASSISTÊNCIA COGNITIVA
-------------------------------------------------------------------------------

Modelos de linguagem podem auxiliar na extração e organização estrutural
dos sistemas discursivos, sem criação automática de estados nem validação
científica.

===============================================================================
PARTE II — RECOMMENDATION TRAJECTORY CONTEXT (C)
===============================================================================

-------------------------------------------------------------------------------
8. DEFINIÇÃO DO BOUNDED CONTEXT
-------------------------------------------------------------------------------

Nome:
RecommendationTrajectoryContext

Tipo:
Bounded Context Observacional e Analítico

Responsabilidade Central:
Registrar e analisar trajetórias de recomendações técnicas,
permitindo comparação temporal e leitura paralela com trajetórias
espaciais e ecológicas.

-------------------------------------------------------------------------------
9. PRINCÍPIOS INEGOCIÁVEIS
-------------------------------------------------------------------------------

1) Recomendações não causam efeitos automaticamente.
2) Efeitos são apenas observados em paralelo.
3) Trajetórias são comparativas, não explicativas.
4) Nenhuma recomendação gera decisão no sistema.

-------------------------------------------------------------------------------
10. ENTIDADES E AGREGADOS
-------------------------------------------------------------------------------

Entity: RecommendationSnapshot
Identidade: RecommendationSnapshotID

Atributos:
- RecommendationText
- ContextConditions
- IntendedAction
- ExpectedOutcome
- SourceReference
- TemporalContext

Aggregate Root: RecommendationTrajectory
- Sequência ordenada de RecommendationSnapshots
- Metadados da trajetória

-------------------------------------------------------------------------------
11. RELAÇÃO COM OUTROS CONTEXTOS
-------------------------------------------------------------------------------

Dependências Permitidas:
- NarrativeObservationContext
- FourthDimensionSystem
- PatchTrajectoryAnalysis

Dependências Proibidas:
- Core Domain
- Uso da Terra
- Simulação
- Feedback automático

-------------------------------------------------------------------------------
12. NOTA FINAL
-------------------------------------------------------------------------------

Estes contextos ampliam a legibilidade científica do STRATA,
tornando explícitos sistemas implícitos de discurso e recomendação,
sem comprometer a integridade epistemológica do sistema.

O STRATA observa.
O pesquisador interpreta.
A causalidade permanece externa.

===============================================================================
FIM DO DOCUMENTO
===============================================================================
