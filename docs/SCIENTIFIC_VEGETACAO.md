# Vegetação — Detalhamento Científico

Este documento descreve a funcionalidade científica de vegetação no
SisterSTRATA, com foco em escopo, entradas, saídas e limites.
Ele amplia a legibilidade do sistema, não sua autoridade científica.

## 1. Escopo científico
- Representar hipóteses de vegetação potencial.
- Vincular vegetação a condições ambientais e tempo.
- Servir como base para cenários e análises de trajetória.

## 2. Modelo conceitual
- Autômato celular com estados de vigor por célula.
- Sucessão ecológica por transições discretas.
- Dependência de água e nutrientes do solo.

## 3. Entradas (Input)
- Hipóteses declaradas de vegetação potencial
- Condições de solo e hidrologia
- Tempo e parâmetros de sucessão

## 4. Estado derivado (State)
- Vigor vegetal por célula
- Estágio sucessional
- Distribuição espacial de classes

## 5. Observáveis (Output)
- Mapas e legendas de classes de vegetação
- Cenários persistidos para análise temporal

## 6. Limitações e salvaguardas
- Vegetação declarada é hipótese explícita.
- Não substitui validação ecológica externa.
- Visualização não altera o estado derivado.

## 7. Referências do projeto
- [Fundação Científica](SISTERSTRATA_SCIENTIFIC_FOUNDATION.md)
- [DDD Vegetação](../DDD_VegetationSystemOriginal_STRATA.txt)
- [Funcionalidades Científicas](SCIENTIFIC_FUNCTIONALITIES.md)
