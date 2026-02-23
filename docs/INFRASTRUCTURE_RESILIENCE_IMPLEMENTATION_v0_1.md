# Infrastructure Resilience v0.1

## Objetivo

Registrar o que foi implementado no SisterSTRATA para o InfrastructureLayer v0.1, como executar, onde os artefatos aparecem no workspace e como interpretar os quantitativos energeticos.

## Base documental (DDD)

- `../STRATA_DDD_InfrastructureLayer_v0_1.md`
- `../STRATA_DDD_SharedEnergyContext_v0_1.md`
- `../STRATA_DDD_IdentityResilienceContext_v0_1.md`
- `../STRATA_ScientificInstrumentationContext_DDD_MVP.md` (futuro, nao implementado no core v0.1)

## Escopo implementado

### Core Domain (InfrastructureLayer v0.1)

- SharedEnergyContext com pool energetico territorial:
  - `src/core/domain/energy/EnergyPool.hpp`
  - `src/core/domain/energy/EnergyAllocationPolicy.hpp`
  - `src/core/domain/energy/EnergyAllocationPolicy.cpp`
- Orquestracao diaria entre energia, identidade e SETO:
  - `src/core/domain/infrastructure/InfrastructureTypes.hpp`
  - `src/core/domain/infrastructure/InfrastructureOrchestrator.hpp`
  - `src/core/domain/infrastructure/InfrastructureOrchestrator.cpp`
- Simulador deterministico diario e exportacao CSV:
  - `src/core/domain/simulation/EnvironmentController.hpp`
  - `src/core/domain/simulation/EnvironmentController.cpp`
  - Presets de cenario ecologico:
    - `Normal`
    - `SevereDrought` (seca severa forcada para estresse energetico)

### IdentityResilienceContext (FocinhoTrack agregado)

- Perfil energetico por componentes operacionais:
  - `boot_wh_per_day`, `idle_wh_per_day`
  - `sensing_wh_per_event`, `processing_wh_per_event`, `communication_wh_per_event`
- Breakdown separado de energia solicitada vs consumida.
- Estados operacionais: `Full`, `Reduced`, `Survival`, `Suspended`.
- Arquivos:
  - `src/core/domain/identity/IdentityNode.hpp`
  - `src/core/domain/identity/IdentityNode.cpp`

### SoilElectricalResilienceContext (SETO agregado)

- Perfil energetico com base fixa + parte dinamica por umidade.
- Breakdown separado de energia solicitada vs consumida.
- Estados operacionais: `Full`, `Reduced`, `Survival`, `Suspended`.
- Arquivos:
  - `src/core/domain/soil/SoilMonitorNode.hpp`
  - `src/core/domain/soil/SoilMonitorNode.cpp`

### Application Layer

- `Session` aceita execucao baseline e execucao configurada:
  - `runInfrastructureResilienceSimulation(int days)`
  - `runInfrastructureResilienceSimulation(const InfrastructureEvaluationConfig&)`
- Relatorio JSON enriquecido com:
  - `runConfig.identity.profile`
  - `runConfig.soil.profile`
  - `runConfig.ftHardware`
  - `finalState.identity/soil` com requested, allocated, consumed e breakdowns
- Arquivos:
  - `src/application/Session.hpp`
  - `src/application/Session.cpp`

### UI (Analysis Workspace)

- Nova montagem de FT por componentes no tab `Infraestrutura`:
  - modulo de compute
  - modulo de sensor
  - modulo de radio
  - eventos por animal por dia
  - quantidade de FT para custo
- Botao para submeter configuracao FT e botao baseline.
- Seletor de cenario ecologico no painel:
  - `Normal`
  - `Seca Severa`
- Exibicao do resumo do ultimo relatorio (pool, identity, SETO, artefatos).
- Arquivos:
  - `src/ui/panels/AnalysisWorkspacePanel.hpp`
  - `src/ui/panels/AnalysisWorkspacePanel.cpp`

### Testes e build

- Conflito de alvo CMake corrigido:
  - `tests/core/CMakeLists.txt`: `core_infrastructure_tests`
- Testes de infraestrutura adicionados/atualizados:
  - `tests/application/InfrastructureResilienceRunTest.cpp`
  - `tests/core/TestInfrastructure.cpp`
  - `tests/application/CMakeLists.txt`
  - `tests/core/CMakeLists.txt`
- Core passou a compilar os novos arquivos:
  - `src/core/CMakeLists.txt`

## Como executar

```bash
cmake -S . -B build
cmake --build build
./build/bin/application_mapper_tests --gtest_filter=InfrastructureResilienceRunTest.*
./build/bin/core_infrastructure_tests
```

## Onde os resultados aparecem no workspace

Dentro do `projectRoot` ativo da sessao:

- CSV latest:
  - `assets/data/user_db/reports/infrastructure/InfrastructureSimulation.latest.csv`
- CSV com timestamp:
  - `assets/data/user_db/reports/infrastructure/InfrastructureSimulation_YYYYMMDD_HHMMSS.csv`
- JSON latest:
  - `assets/data/user_db/reports/infrastructure/InfrastructureResilience.latest.json`
- JSON com timestamp:
  - `assets/data/user_db/reports/infrastructure/InfrastructureResilience_YYYYMMDD_HHMMSS.json`

## Como interpretar quantitativos energeticos

- Unidade principal no relatorio: `Wh` (energia), nao `W` (potencia).
- Conversao: `kWh = Wh / 1000`.
  - Exemplo: `2005 Wh = 2.005 kWh`.
  - Exemplo: `3515.62 Wh = 3.51562 kWh`.

Campos importantes:

- `requestedWh`: demanda calculada do no no dia.
- `allocatedWh`: energia entregue pela politica de alocacao.
- `consumedWh`: energia efetivamente usada apos estado operacional.

No v0.1, `Identity` representa o bloco agregado do FocinhoTrack no territorio (nao multiploes nodos fisicos detalhados). `SETO` representa o bloco agregado de monitoramento eletrico do solo.

## Cenarios ecologicos (v0.1)

O cenario da execucao e salvo em `runConfig.ecologicalScenario` no JSON:

- `normal_deterministic_v0_1`: sazonalidade deterministica padrao.
- `severe_drought_v0_1`: reduz geracao solar e umidade para forcar condicao critica energetica.

Objetivo: permitir stress test causal (por condicao ambiental), sem introduzir ruido estocastico nesta etapa.

## Sobre arquivos "repetidos"

`*.latest.*` e `*_<timestamp>.*` podem ter o mesmo conteudo quando saem da mesma execucao. Isso e esperado:

- `latest`: ponteiro para o resultado mais recente.
- `timestamp`: historico versionado por execucao.

## Limites atuais (intencionais no v0.1)

- Modelo sistemico energetico territorial.
- Nao e simulacao eletrica detalhada de hardware.
- Sem dinamica intra-dia por fases (boot/idle/capture/process/tx/sleep ao longo do dia).
- Sem modelo fisico de tensao/ruido/latencia termica.

Esses pontos ficam para evolucao posterior sem quebrar o contrato do DDD v0.1.
