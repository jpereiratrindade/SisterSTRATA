Domain-Driven Design (DDD)

STRATA — SharedEnergyContext v0.1

Modelo Territorial com Pool Energético Igualitário

------------------------------------------------------------------------

1. Objetivo

Modelar a disponibilidade energética territorial compartilhada entre
múltiplas infraestruturas (FocinhoTrack e SETO), utilizando um Pool
Energético Único com alocação igualitária.

Este modelo é simplificado (v0.1) e serve como base para implementação
inicial no STRATA.

------------------------------------------------------------------------

2. Escopo do Modelo

-   Um único Pool Energético Territorial
-   Múltiplas infraestruturas consumidoras
-   Alocação igualitária de energia disponível
-   Atualização diária (step temporal discreto)

Não modela ainda: - Perdas elétricas detalhadas - Baterias individuais
por nó - Estratégias prioritárias

------------------------------------------------------------------------

3. Entidades Principais

3.1 EnergyPool

Representa a energia total disponível no território.

Campos: - total_capacity_wh - current_storage_wh - daily_generation_wh -
daily_consumption_wh

------------------------------------------------------------------------

3.2 EnergyAllocationPolicy

Tipo atual: - Equalitarian

Função: Distribuir energia disponível igualmente entre infraestruturas
ativas.

------------------------------------------------------------------------

3.3 InfrastructureEnergyState

Para cada infraestrutura:

Campos: - requested_energy_wh - allocated_energy_wh - operational_state

------------------------------------------------------------------------

4. Entradas do EcologicalLayer

Recebe diariamente:

-   Insolação diária (Wh/m² convertida em geração)
-   Temperatura média
-   Sazonalidade (opcional futuro)

A geração diária é calculada como:

daily_generation_wh = insolacao * eficiencia_sistema

------------------------------------------------------------------------

5. Processo Diário (Step Temporal)

1.  Atualizar geração diária.
2.  Atualizar armazenamento: current_storage += daily_generation
3.  Receber demandas energéticas das infraestruturas.
4.  Calcular energia disponível para alocação.
5.  Aplicar política igualitária: allocated_energy = current_storage /
    numero_de_infraestruturas
6.  Atualizar armazenamento: current_storage -= soma(allocated_energy)
7.  Atualizar estados operacionais.

------------------------------------------------------------------------

6. Estados Energéticos Territoriais

Baseado no percentual de armazenamento:

Abundante: - storage > 70%

Estável: - 40% < storage ≤ 70%

Restrito: - 20% < storage ≤ 40%

Crítico: - storage ≤ 20%

------------------------------------------------------------------------

7. Estados Operacionais por Infraestrutura

Se allocated_energy ≥ requested_energy: - Operacional Pleno

Se allocated_energy < requested_energy e > 0: - Operação Reduzida

Se allocated_energy = 0: - Suspensão Temporária

------------------------------------------------------------------------

8. Métricas de Resiliência

-   Tempo médio em estado Crítico
-   Frequência de suspensão por infraestrutura
-   Energia mínima residual
-   Percentual de demanda não atendida
-   Tempo médio de recuperação

------------------------------------------------------------------------

9. Eventos do Sistema

-   SolarInputUpdated
-   EnergyDepleted
-   EnergyRecovered
-   AllocationPerformed
-   InfrastructureSuspended

------------------------------------------------------------------------

10. Restrições Arquiteturais

-   SharedEnergyContext é autoridade única sobre energia territorial.
-   Nenhuma infraestrutura pode consumir energia diretamente.
-   Alocação deve ocorrer apenas via EnergyAllocationPolicy.
-   Não existem dependências circulares.

------------------------------------------------------------------------

11. Limitações Conhecidas (v0.1)

-   Pool único simplificado.
-   Não modela baterias locais individuais.
-   Não considera eficiência variável por temperatura.
-   Não modela perdas de conversão detalhadas.

Evoluções futuras podem incluir: - Baterias distribuídas - Prioridades
adaptativas - Mercado energético interno - Estratégia territorial de
sobrevivência

------------------------------------------------------------------------

Fim do Documento
