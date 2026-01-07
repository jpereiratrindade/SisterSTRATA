# Hidrologia — Detalhamento Científico

Este documento descreve a funcionalidade científica de hidrologia no
SisterSTRATA, com foco em escopo, entradas, saídas e limites.
Ele amplia a legibilidade do sistema, não sua autoridade científica.

## 1. Escopo científico
- Modelar a dinâmica de água superficial e subsuperficial.
- Calcular drenagem, acúmulo de fluxo e métricas associadas.
- Relacionar hidrologia com solo e vegetação.

## 2. Princípios e regras
- Conservação de massa.
- Fluxo superficial por potencial gravitacional.
- Infiltração como interação com o solo.
- Drenagem D8 e propagação topológica.

## 3. Entradas (Input)
- Relevo (declividade, direção de fluxo)
- Clima (chuva)
- Parâmetros hidrológicos do solo
- Tempo de simulação

## 4. Estado derivado (State)
- Lâmina d’água
- Velocidade de fluxo
- Taxa de infiltração
- Acúmulo de fluxo por célula

## 5. Observáveis (Output)
- Mapas de drenagem e TWI
- Segmentação de bacias
- Relatórios hidrológicos exportáveis

## 6. Limitações e salvaguardas
- Sem parâmetros fisicamente impossíveis.
- Visualização não altera o estado derivado.
- Resultados exigem validação científica externa.

## 7. Referências do projeto
- `docs/SISTERSTRATA_SCIENTIFIC_FOUNDATION.md`
- `docs/SCIENTIFIC_FUNCTIONALITIES.md`
