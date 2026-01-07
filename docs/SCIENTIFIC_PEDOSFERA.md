# Pedosfera (SCORPAN/SiBCS) — Detalhamento Científico

Este documento descreve a funcionalidade científica de pedosfera no
SisterSTRATA, com foco em escopo, entradas, saídas e limites.
Ele amplia a legibilidade do sistema, não sua autoridade científica.

## 1. Escopo científico
- Modelar o solo como função causal do SCORPAN.
- Classificar o solo via regras determinísticas (SiBCS).
- Garantir rastreabilidade de hipóteses até a visualização.

## 2. Modelo conceitual
- Equação base: `S = f(S, C, O, R, P, A, N)`
- Variáveis:
  - S (Solo pré-existente)
  - C (Clima)
  - O (Organismos)
  - R (Relevo)
  - P (Material de origem)
  - A (Tempo)
  - N (Espaço/posição)

## 3. Entradas (Input)
- Clima (precipitação, temperatura)
- Geologia/litologia
- Relevo (declividade, curvatura)
- Tempo de simulação
- Intervenções explícitas (se aplicável)

## 4. Estado derivado (State)
- Propriedades do solo (umidade, pH, carbono orgânico, compactação)
- Classes pedológicas inferidas por regras SiBCS

## 5. Observáveis (Output)
- Visualização 3D do solo
- Relatórios e probes quantitativos
- Mapas temáticos por variável

## 6. Limitações e salvaguardas
- Parâmetros fisicamente impossíveis devem ser bloqueados.
- Cenários ecológicos improváveis devem gerar alerta.
- Visualização não altera o estado derivado.

## 7. Referências do projeto
- `docs/SISTERSTRATA_SCIENTIFIC_FOUNDATION.md`
- `docs/SCIENTIFIC_FUNCTIONALITIES.md`
