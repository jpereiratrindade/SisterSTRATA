===============================================================================
SISTERSTRATA – EXTENSÃO DE PATCH ANALYSIS
TRAJETÓRIA MULTI-ESTADOS (FOURTH DIMENSION)
===============================================================================

VERSÃO: 1.0
STATUS: PROPOSTA ARQUITETURAL (APTA PARA IMPLEMENTAÇÃO)
ESCOPO: Patch Analysis / Resiliência / Dinâmica Espaço-Temporal

===============================================================================
1. PRINCÍPIO FUNDAMENTAL
===============================================================================

A análise de patches no SisterSTRATA NÃO deve se restringir à comparação
binária entre dois estados (t₀ → t₁).

O sistema deve permitir a análise de UMA SEQUÊNCIA ARBITRÁRIA DE ESTADOS
TEMPORAIS, interpretados como uma TRAJETÓRIA.

O tempo é tratado como dimensão explícita do domínio, e não como exceção
analítica.

===============================================================================
2. CONCEITO-CHAVE: TRAJETÓRIA DE PATCH
===============================================================================

Define-se como TRAJETÓRIA DE PATCH a sequência ordenada de estados
espaciais de um mesmo patch ao longo do tempo.

Um patch deixa de ser apenas uma entidade geométrica e passa a ser um
objeto histórico, com:

- origem
- duração
- transformações
- destino (persistência, fusão, fragmentação ou extinção)

===============================================================================
3. MODELO CONCEITUAL (ALTO NÍVEL)
===============================================================================

Patch
 └── PatchState (t0)
 └── PatchState (t1)
 └── PatchState (t2)
 └── ...
 └── PatchState (tn)

A quantidade de estados NÃO é limitada.
A trajetória pode conter tantos estados quantos forem fornecidos
pelo usuário ou pelo sistema.

===============================================================================
4. GRUPOS DE MÉTRICAS DE PATCH (PATCHSTATE)
===============================================================================

Cada PatchState pode conter métricas organizadas por PAPEL ECOLÓGICO.

-------------------------------------------------------------------------------
4.1 GEOMETRIA E FORMA
-------------------------------------------------------------------------------
- Area
- Perimeter
- Shape Index
- Fractal Dimension Index
- Radius of Gyration

-------------------------------------------------------------------------------
4.2 BORDA E ÁREA NÚCLEO
-------------------------------------------------------------------------------
- Edge Length
- Edge Density
- Core Area
- Core Area Index
- Edge Contrast Index (quando aplicável)

-------------------------------------------------------------------------------
4.3 CONTEXTUALIZAÇÃO ESPACIAL
-------------------------------------------------------------------------------
- Nearest Neighbor Distance
- Proximity Index
- Adjacency (por classe)
- Interspersion & Juxtaposition Index

-------------------------------------------------------------------------------
4.4 CONECTIVIDADE E COESÃO
-------------------------------------------------------------------------------
- Patch Cohesion Index
- Connectivity Class
- Isolation Class

===============================================================================
5. MÉTRICAS DE TRAJETÓRIA (PATCHTRAJECTORY)
===============================================================================

Derivadas da análise de MÚLTIPLOS PatchStates ao longo do tempo.

-------------------------------------------------------------------------------
5.1 EXISTÊNCIA E PERSISTÊNCIA
-------------------------------------------------------------------------------
- Lifespan (número de estados)
- Persistence Ratio
- Birth Time
- Death Time

-------------------------------------------------------------------------------
5.2 PROCESSOS DOMINANTES
-------------------------------------------------------------------------------
- Split Count
- Merge Count
- Fragmentation Frequency
- Coalescence Frequency

-------------------------------------------------------------------------------
5.3 DINÂMICA E TENDÊNCIA
-------------------------------------------------------------------------------
- Net Area Trend (ganho / perda / estável)
- Shape Volatility Index
- Structural Stability Index
- Dominant Trajectory Type

===============================================================================
6. TIPOS DE TRAJETÓRIA (CLASSIFICAÇÃO SEMÂNTICA)
===============================================================================

Uma trajetória pode ser classificada, por exemplo, como:

- Estável
- Erosiva
- Fragmentante
- Reorganizativa
- Pulsátil
- Transitória

Essa classificação NÃO é determinística e pode coexistir
com múltiplas interpretações.

===============================================================================
7. RELAÇÃO COM RESILIÊNCIA
===============================================================================

A resiliência NÃO é inferida apenas por estabilidade de área,
mas pela capacidade do sistema de:

- manter conectividade funcional
- reorganizar estrutura sem colapso
- absorver perturbações ao longo da trajetória

A análise de trajetória é o principal insumo para interpretações
de resiliência no SisterSTRATA.

===============================================================================
8. INTEGRAÇÃO COM ASSISTÊNCIA COGNITIVA (LLM)
===============================================================================

O sistema de IA NÃO recebe estados brutos.

Ele recebe SUMÁRIOS SEMÂNTICOS de trajetória, por exemplo:

PATCH_TRAJECTORY_SUMMARY
- lifespan: longo
- dominant_process: fragmentação
- volatility: alta
- net_area_trend: estável
- structural_stability: baixa

A IA atua como OBSERVADOR INTERPRETATIVO, nunca como decisor.

===============================================================================
9. DIRETRIZES DE IMPLEMENTAÇÃO
===============================================================================

- O número de estados temporais deve ser arbitrário.
- O modelo deve funcionar com 2 ou com 200 estados.
- PatchTrajectory não altera PatchState.
- Métricas de trajetória são derivadas, nunca armazenadas como estado bruto.
- O Core Domain permanece livre de dependências de IA.

===============================================================================
FIM DO DOCUMENTO
===============================================================================
