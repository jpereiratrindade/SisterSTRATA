# Pedosfera (SCORPAN / SiBCS) — Fundamentação Científica

Este documento apresenta a fundamentação científica do domínio Pedosfera no
SisterSTRATA. Seu objetivo é explicitar os modelos conceituais adotados, as
escolhas metodológicas realizadas e os limites epistemológicos assumidos pelo
sistema.

O documento visa ampliar a legibilidade científica do domínio, não atribuir
autoridade normativa nem substituir literatura pedológica especializada.

---

## 1. Posicionamento científico do domínio

No SisterSTRATA, a pedosfera é tratada como um componente estruturante dos
sistemas socioecológicos, atuando como meio físico, reserva funcional e
registro histórico de processos ambientais.

O solo não é modelado como entidade observada diretamente nem como resultado
determinístico pleno, mas como um **estado inferido**, construído a partir de
funções ecológicas reconhecidas e explicitamente delimitadas.

Essa abordagem permite integrar o solo a análises espaciais e temporais sem
atribuir ao modelo capacidades explicativas além de seu escopo científico.

---

## 2. Fundamentos da pedologia funcional

A pedologia moderna reconhece o solo como um corpo natural dinâmico, resultante
da interação histórica entre clima, organismos, relevo, material de origem,
tempo e posição na paisagem.

Em escalas espaciais amplas e em contextos de modelagem computacional, a
observação direta e completa desses processos torna-se impraticável. Nesses
casos, funções pedológicas atuam como **estruturas heurísticas**, organizando
relações plausíveis entre fatores ambientais e propriedades do solo.

O SisterSTRATA adota essa perspectiva funcional, priorizando coerência
científica, rastreabilidade de hipóteses e clareza epistemológica.

---

## 3. O modelo SCORPAN: uso e limites

O domínio Pedosfera baseia-sebi na formulação SCORPAN, expressa como:

S = f(S, C, O, R, P, A, N)

onde:

- S — Solo pré-existente (condição inicial inferida)
- C — Clima
- O — Organismos
- R — Relevo
- P — Material de origem
- A — Tempo
- N — Espaço ou posição na paisagem

No contexto do SisterSTRATA, o SCORPAN é utilizado como uma **função ecológica
inferencial**, e não como um modelo mecanístico de pedogênese.

A relação expressa pela função não implica causalidade direta entre fatores,
mas orienta a construção de estados de solo plausíveis e comparáveis entre
cenários.

---

## 4. Tratamento do tempo no domínio Pedosfera

O fator A (Tempo) não é interpretado como cronologia pedogenética absoluta.

No STRATA, o tempo assume a forma de:
- índice relativo
- marcador de estado
- referência comparativa entre cenários

Essa escolha é consistente com a separação entre simulação dinâmica e análise
observacional, bem como com o FourthDimensionSystem, no qual o tempo emerge
como sequência de estados discretos.

---

## 5. Classificação pedológica e SiBCS

A classificação do solo no SisterSTRATA baseia-se em regras determinísticas
inspiradas no Sistema Brasileiro de Classificação de Solos (SiBCS).

A SiBCS é compreendida como um **sistema classificatório**, e não como um modelo
causal. Suas regras são utilizadas como instrumento formal de interpretação
pedológica, respeitando suas limitações conhecidas, especialmente em ambientes
campestres e mosaicos pedológicos complexos.

As classes inferidas não representam levantamentos de campo, mas categorias
operacionais para análise espacial e integração entre domínios.

---

## 6. Entradas do sistema (Inputs)

As principais entradas consideradas incluem:
- Clima (ex.: precipitação, temperatura)
- Geologia e litologia
- Relevo (declividade, curvatura, posição topográfica)
- Índices temporais relativos (quando aplicável)
- Intervenções explícitas e documentadas (quando habilitadas)

Todas as entradas são submetidas a validação de coerência física e ecológica.

---

## 7. Estado derivado do solo

A partir das entradas, o sistema constrói um **estado derivado de solo**, que pode
incluir:
- propriedades físicas e químicas (ex.: umidade, pH, carbono orgânico,
  compactação)
- classes pedológicas inferidas por regras determinísticas

Esse estado constitui uma **construção científica operacional**, adequada para
simulação integrada e análise comparativa, sem pretensão de equivalência direta
com o solo real observado.

---

## 8. Observáveis e visualização

Os principais observáveis produzidos pelo domínio incluem:
- visualização 3D de propriedades e classes de solo
- mapas temáticos por variável pedológica
- relatórios e sondagens quantitativas (probes)

A visualização é estritamente passiva, não alterando o estado derivado do solo
nem introduzindo retroalimentações no domínio científico.

---

## 9. Limitações epistemológicas e salvaguardas

O domínio Pedosfera incorpora salvaguardas explícitas, incluindo:
- bloqueio de parâmetros fisicamente impossíveis
- geração de alertas para cenários ecologicamente improváveis
- separação rigorosa entre estado derivado, visualização e interpretação

Inferências normativas, prescrições de manejo ou avaliações de desempenho
ecossistêmico não são realizadas neste domínio.

---

## 10. Relação com outros domínios do STRATA

O domínio Pedosfera fornece condições estruturais para outros domínios, sem
exercer papel decisório, incluindo:
- Hidrologia
- VegetationSystemOriginal
- FourthDimensionSystem

O solo atua como base biofísica, nunca como agente controlador.

---

## 11. Documentos relacionados

A fundamentação teórica aprofundada, a revisão bibliográfica e as discussões
epistemológicas associadas ao domínio Pedosfera estão documentadas em:
- SISTERSTRATA_SCIENTIFIC_FOUNDATION.md
- SCIENTIFIC_FUNCTIONALITIES.md
- Documento específico de revisão: SCIENTIFIC_PEDOSFERA_BIBLIO.md