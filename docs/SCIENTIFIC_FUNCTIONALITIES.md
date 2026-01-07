# Funcionalidades Científicas, Aplicações e Limitações

Este documento resume as funcionalidades científicas do SisterSTRATA, suas
aplicações no código e as limitações explicitadas na documentação do projeto.
Ele amplia a legibilidade do sistema, não sua autoridade científica.

## 1. Funcionalidades científicas (Fundação)

### 1.1 Pedosfera (Solo)
- Modelo causal baseado em SCORPAN.
- Classificação determinística via SiBCS.
- Cada estado de solo deriva de causas explicáveis.

Referências: [Fundação Científica](SISTERSTRATA_SCIENTIFIC_FOUNDATION.md), [Pedosfera](SCIENTIFIC_PEDOSFERA.md)

### 1.2 Hidrologia
- Conservação de massa.
- Fluxo superficial, infiltração, drenagem D8 e acumulação de fluxo.
- Bacias, TWI e relatórios exportáveis.

Referências: [Fundação Científica](SISTERSTRATA_SCIENTIFIC_FOUNDATION.md), [Hidrologia](SCIENTIFIC_HIDROLOGIA.md)

### 1.3 Vegetação
- Autômato celular com vigor por célula.
- Sucessão ecológica baseada em tempo e condições ambientais.

Referências: [Fundação Científica](SISTERSTRATA_SCIENTIFIC_FOUNDATION.md), [DDD Vegetação](../DDD_VegetationSystemOriginal_STRATA.txt), [Vegetação](SCIENTIFIC_VEGETACAO.md)

### 1.4 Quarta Dimensão (Resiliência)
- TimeSlices imutáveis.
- Métricas de coerência e trajetória de patches.

Referências: [Fundação Científica](SISTERSTRATA_SCIENTIFIC_FOUNDATION.md), [DDD Quarta Dimensão](../DDD_FourthDimensionSystem.txt), [DDD Trajetória de Patches](../DDD_PatchTrajectory_Analysis.txt), [Arquitetura Vetorial](Arquitetura_Vetorial_Hipoteses.md), [Quarta Dimensão](SCIENTIFIC_QUARTA_DIMENSAO.md)

### 1.5 Contrato Input-State-Output
- Separação entre causa (input), estado derivado e observável.
- Visualização nunca altera simulação.

Referências: [Fundação Científica](SISTERSTRATA_SCIENTIFIC_FOUNDATION.md)

## 2. Aplicações no código (UI e fluxos)

### 2.1 Solo e SCORPAN/SiBCS
- Simulação de solo e integração com análise de patches.
Referências: [SoilSimPanel](../src/ui/panels/SoilSimPanel.cpp)

### 2.2 Análise de patches
- Cálculo de métricas e leitura de legenda.
Referências: [PatchAnalysisPanel](../src/ui/panels/PatchAnalysisPanel.cpp), [DDD Trajetória de Patches](../DDD_PatchTrajectory_Analysis.txt)

### 2.3 Hidrologia
- Parâmetros e exibição de resultados hidrológicos.
Referências: [HydrologyPanel](../src/ui/panels/HydrologyPanel.cpp)

### 2.4 Linha do tempo e trajetórias
- Visualização temporal, comparações e exportação.
Referências: [TimelinePanel](../src/ui/panels/TimelinePanel.cpp), [DDD Quarta Dimensão](../DDD_FourthDimensionSystem.txt)

### 2.5 Hipóteses e cenários de vegetação
- Declaração de hipóteses e persistência de cenários.
Referências: [VegetationDeclarationPanel](../src/ui/panels/VegetationDeclarationPanel.cpp), [DDD Vegetação](../DDD_VegetationSystemOriginal_STRATA.txt)

### 2.6 Geração e exportação de terreno
- Geração procedural e exportação de malhas.
Referências: [TerrainGeneratorPanel](../src/ui/panels/TerrainGeneratorPanel.cpp)

## 3. Limitações e salvaguardas

### 3.1 Limites físicos e ecológicos
- Parâmetros fisicamente impossíveis devem ser bloqueados.
- Cenários extremos devem gerar alerta.

Referência: [Fundação Científica](SISTERSTRATA_SCIENTIFIC_FOUNDATION.md)

### 3.2 Integridade epistemológica (LLM/CAC)
- IA é auxiliar e não altera o Core Domain.
- Não decide, não simula, não prescreve ações.
- Respostas devem declarar limites e incertezas.

Referência: [DDD Assistência Cognitiva](../DDD_Cognitive_Assistance_Qwen_STRATA_v1.0.txt)

### 3.3 Separação entre simulação e visualização
- Mudanças visuais não alteram estados de simulação.

Referência: [Fundação Científica](SISTERSTRATA_SCIENTIFIC_FOUNDATION.md)

## 4. Documentos base
- [Fundação Científica](SISTERSTRATA_SCIENTIFIC_FOUNDATION.md)
- [DDD Assistência Cognitiva](../DDD_Cognitive_Assistance_Qwen_STRATA_v1.0.txt)
- [Funcionalidades Propostas](STRATA_Funcionalidades_Propostas_v1.txt)
- [README](../README.md)
