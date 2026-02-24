# Memorando Estrategico Interno
## Inicio da Fase F1 - Scientific Hardening

Data: 2026-02-24  
Projeto: STRATA  
Decisao Institucional: Inicio formal da Fase F1

---

## 1. Declaracao

STRATA passa a operar sob regime de hardening cientifico.

A partir desta data:

- nenhuma nova feature entra no Core Domain durante F1;
- FT e SETO evoluem apenas sob regras de blindagem arquitetural;
- prioridade operacional passa a ser enforcement de invariantes cientificos.

Este memorando referencia `ADR-005`.

---

## 2. Objetivo da Fase F1

Converter principios arquiteturais em invariantes executaveis:

- determinismo auditavel,
- membrana epistemologica executavel,
- assinatura cientifica por execucao,
- gates de teste vinculados aos ADRs criticos.

---

## 3. Politica de Freeze (F1)

Durante F1:

- proibida adicao de novos modulos no core;
- proibida expansao funcional da camada 4D;
- proibida integracao causal profunda de SETO no nucleo ecologico;
- permitido apenas trabalho de hardening e governanca executavel.

---

## 4. Criterios Objetivos de Conclusao

F1 conclui somente quando:

1. duas execucoes com mesma seed/config/binario geram mesmo state hash;
2. violacao de membrana gera falha automatizada;
3. cada execucao gera assinatura cientifica rastreavel;
4. criterios de aceite de `ADR-004` sao comprovados por testes.

---

## 5. Regra de Regressao Arquitetural

A partir do commit que introduz `ADR-005`, toda alteracao que viole:

- `ADR-001` (infraestrutura como evidencia), ou
- `ADR-002` (determinismo do core),

deve ser tratada como regressao arquitetural.
