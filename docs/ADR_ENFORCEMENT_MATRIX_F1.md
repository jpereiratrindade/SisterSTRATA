# Matriz ADR -> Enforcement Tecnico (F1)

Escopo: hardening da fase F1  
Referencia institucional: `adr/ADR-005_Scientific_Hardening_Phase_F1.md`

---

| ADR | Principio | Enforcement requerido | Gate automatizado minimo |
| --- | --- | --- | --- |
| ADR-001 | Infraestrutura como eixo de evidencia | Proibir mutacao direta do estado ecologico por infraestrutura | Falhar teste quando houver caminho infra -> mutacao ecologica |
| ADR-002 | Determinismo Tier 1 | Seed explicita, tier declarado, state hash deterministico | Replay test com hash identico sob mesmo binario/config/seed |
| ADR-004 | Membrana O->I->4D | DTO observacional read-only + portas unidirecionais | Falhar teste quando houver feedback proibido |
| ADR-005 | Regime F1 e freeze | Bloqueio de features no core durante fase | Revisao/CI falha se PR introduzir feature fora do escopo F1 |

---

## Criterio de aceite por ADR (resumo)

1. ADR-001
- nenhuma dependencia que permita escrita ecologica indevida por infraestrutura.

2. ADR-002
- metadados de determinismo emitidos por execucao:
  - seed
  - determinismTier
  - entropySources
  - stateHash

3. ADR-004
- contrato de membrana testado e protegido por CI.

4. ADR-005
- freeze de feature respeitado ate encerramento formal da fase F1.
