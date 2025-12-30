# SisterSTRATA

**Plataforma Científica de Processamento de Dados e Visualização 3D**

SisterSTRATA é um instrumento científico digital projetado para a simulação de ecossistemas complexos. Ele integra modelos de pedosfera (Solo), hidrologia e vegetação em um ambiente 3D de alta performance, focando na consistência causal e integridade científica.

## Documentação Fundamental

Para entender a filosofia, a governança dos modelos e a arquitetura de conhecimento que rege este projeto, consulte:

*   **[Fundação Científica (Scientific Foundation)](docs/SISTERSTRATA_SCIENTIFIC_FOUNDATION.md)**: O documento "constitucional/white paper" que define as leis físicas, o contrato de pipeline (Input/Derived/Observable) e a estrutura dos domínios científicos.
*   **[Diretrizes de Uso da Terra](SISTERSTRATA_Diretrizes_Uso_da_Terra_Vetores_Processo_v1.0.txt)**: Definição de vetores de processo e emergência do uso da terra.
*   **[Diretrizes de Resiliência](STRATA_Diretrizes_Resiliencia_Espaco_Tempo_v1.0.txt)**: Princípios de resiliência e análise espaço-temporal.
*   **[Domain-Driven Design (DDD)](projeto_app_ddd_v10.txt)**: A arquitetura de software e organização do código.
*   **[Arquitetura do Sistema](ARCHITECTURE.md)**: Detalhes sobre a implementação técnica em C++ e Vulkan.

## Build e Instalação

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
./SisterSTRATA
```

Consulte `ARCHITECTURE.md` para mais detalhes sobre dependências e configuração do ambiente.
