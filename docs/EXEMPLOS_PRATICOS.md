# Exemplos Práticos SisterSTRATA v1.9.3

Este guia fornece exemplos "mão na massa" para testar as principais funcionalidades científicas do sistema usando os dados de amostra incluídos na pasta `examples/data_samples/`.

## Exemplo 1: Hidrologia e Drenagem
**Objetivo**: Verificar a capacidade do sistema de calcular acumulação de fluxo em um terreno (vale sintético).

1.  **Carregar Dados**:
    *   Vá em `File > Open`.
    *   Selecione `examples/data_samples/exemplo_hidrologia_bacia.csv`.
    *   O sistema carregará uma nuvem de pontos formando um "V" (vale).

2.  **Executar Análise**:
    *   Vá em `Tools > Analyze Drainage`.
    *   Verifique se o "Threshold" está baixo (ex: 2 ou 3).
    *   Clique em **Calculate**.

3.  **Resultado Esperado**:
    *   O sistema desenhará uma linha azul no centro do vale, indicando onde a água se acumularia (o "rio").
    *   O painel exibirá estatísticas de fluxo máximo.

---

## Exemplo 2: Análise de Patches (Vegetação)
**Objetivo**: Identificar fragmentos florestais e calcular suas métricas de forma.

1.  **Carregar Dados**:
    *   Vá em `File > Open`.
    *   Selecione `examples/data_samples/exemplo_vegetacao_patches.csv`.
    *   Este arquivo contém códigos: `1` (Verde/Floresta) e `-1` (Marrom/Solo).

2.  **Executar Análise**:
    *   Vá em `Tools > Patch Analysis`.
    *   Na lista suspensa "Class ID", selecione `1` (Floresta).
    *   Clique em **Analyze**.

3.  **Resultado Esperado**:
    *   O sistema identificará quantos grupos isolados de pixels verdes existem.
    *   Você verá métricas como "Total Area" e "Mean Area".
    *   Tente trocar o Class ID para `-1` para analisar o solo exposto.

---

## Exemplo 3: Sistemas Complexos (JSON)
**Objetivo**: Carregar estruturas cognitivas avançadas (NOC, DSC, Trajetórias).

Antes de iniciar:
*   Abra `View > Analysis Workspace`.
*   Use as abas `Contexto Narrativo`, `Contexto Discursivo`, `Trajectory` e `Recomendacoes`.

1.  **Narrative Observation Context (NOC)**:
    *   Arquivo: `examples/data_samples/example_narrative_observation.json`
    *   Este arquivo define uma observação narrativa qualitativa, incluindo autor, intenção e metáfora ("A chuva não é um sinal para voltar ao normal.").
    *   No Workspace: `Contexto Narrativo > Narrative Observation Context`.

2.  **Discursive System Context (DSC)**:
    *   Arquivo: `examples/data_samples/example_discursive_system.json`
    *   Define um sistema de discurso completo com Declaração de Problemas, Ações e Mecanismos Alegados (baseado na lógica Embrapa 2025).
    *   No Workspace: `Contexto Discursivo`.

3.  **Trajetória de Recomendação**:
    *   Arquivo: `examples/data_samples/example_recommendation_trajectory.json`
    *   Contém uma série temporal de recomendações ("Janela de Oportunidade Pós-seca").
    *   No Workspace: `Trajectory` (e síntese em `Recomendacoes`).

---

## Exemplo 4: Simulação de Solo e Configuração
**Objetivo**: Dados científicos profundos.

*   **Solo (Soil Simulation)**:
    *   Arquivo: `examples/data_samples/exemplo_solo_simulacao.csv`
    *   Contém colunas para Carbono, Nitrogênio e Umidade, além das coordenadas XYZ. Ao carregar como `Point Cloud`, você pode visualizar a distribuição espacial de nutrientes.

*   **Declaração de Vegetação**:
    *   Arquivo: `examples/data_samples/exemplo_config_vegetacao.csv`
    *   Define a paleta de cores e os códigos usados nas matrizes de patches (Forest=1, Water=2, Soil=-1).

---

## Exemplo 5: Simulação e Resiliência (Built-in)
**Objetivo**: Testar a geração de cenários de impacto (Desmatamento) e a visualização temporal.

1.  **Preparação**:
    *   Reinicie o sistema ou use um terreno padrão.
    *   Abra o painel **Fourth Dimension**.

2.  **Simular Impacto**:
    *   Clique em `Simulate: Deforestation`.
    *   O sistema gerará um novo estado na Timeline (ex: T1: Deforestation Scenario).

3.  **Visualizar (Ghost Mode)**:
    *   Selecione o estado `T1` na lista.
    *   Clique no botão **View (Ghost)**.
    *   **Resultado**: O terreno 3D mostrará manchas marrons (solo degradado) sobrepostas à vegetação original. Isso confirma que o sistema de visualização e mapeamento espacial está funcionando.

---
**Nota**: Todos os arquivos CSV de exemplo possuem o cabeçalho `# Origin: 0,0,0` para garantir que o sistema os interprete corretamente como grades de dados científicos.
