# SisterSTRATA

**Plataforma Científica de Processamento de Dados e Visualização 3D**

SisterSTRATA é um instrumento científico digital projetado para a simulação de ecossistemas complexos. Ele integra modelos de pedosfera (Solo), hidrologia e vegetação em um ambiente 3D de alta performance, focando na consistência causal e integridade científica.

## Documentação Fundamental

Para entender a filosofia, a governança dos modelos e a arquitetura de conhecimento que rege este projeto, consulte:

*   **[Fundação Científica (Scientific Foundation)](docs/SISTERSTRATA_SCIENTIFIC_FOUNDATION.md)**: O documento "constitucional/white paper" que define as leis físicas, o contrato de pipeline e a estruturação dos domínios.
*   **[Funcionalidades Científicas](docs/SCIENTIFIC_FUNCTIONALITIES.md)**: Resumo das funcionalidades, aplicações e limitações com links detalhados.
*   **[Pedosfera (SCORPAN/SiBCS)](docs/SCIENTIFIC_PEDOSFERA.md)**: Detalhamento científico do domínio de solo.
*   **[Hidrologia](docs/SCIENTIFIC_HIDROLOGIA.md)**: Detalhamento científico do domínio hidrológico.
*   **[Vegetação](docs/SCIENTIFIC_VEGETACAO.md)**: Detalhamento científico do domínio de vegetação.
*   **[Quarta Dimensão (Resiliência)](docs/SCIENTIFIC_QUARTA_DIMENSAO.md)**: Detalhamento científico de trajetória e coerência.
*   **[Manual do Usuário](docs/MANUAL_DO_USUARIO.md)**: Guia operacional das funcionalidades da ferramenta (Simulação, Visualização 3D, Timeline).
*   **[Análise de Trajetória de Patch (DDD)](DDD_PatchTrajectory_Analysis.txt)**: Definição do modelo multi-estado para patches.
*   **[Arquitetura Vetorial de Hipóteses](docs/Arquitetura_Vetorial_Hipoteses.md)**: Formalização matemática do espaço vetorial de parâmetros.
*   **[Fourth Dimension (Resilience)](DDD_FourthDimensionSystem.txt)**: O modelo de resiliência e coerência temporal.
*   **[Contexto de Observação Narrativa](DDD_NarrativeObservationContext.md)**: Sistema observacional para registrar interpretações discursivas do território.
*   **[Architecture & DDD](projeto_app_ddd_v10.txt)**: Organização do código e princípios de design.

## Requisitos de Sistema

- **GPU**: Compatível com Vulkan 1.2+
- **Compilador**: C++17 ou superior
- **AI (Opcional)**: [Ollama](https://ollama.com/) instalado para assistência cognitiva local (recomendado Qwen2.5:14b).

## Build e Instalação

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
./SisterSTRATA
```

Consulte `ARCHITECTURE.md` para mais detalhes sobre dependências e configuração do ambiente.
