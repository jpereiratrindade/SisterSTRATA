# SisterSTRATA

**Plataforma Científica de Processamento de Dados e Visualização 3D**


SisterSTRATA é um instrumento científico digital projetado para a simulação de ecossistemas complexos. Ele integra modelos de pedosfera (Solo), hidrologia e vegetação em um ambiente 3D de alta performance, focando na consistência causal e integridade científica.

## Principais Funcionalidades (v1.10.6)

*   **Quarta Dimensão (Resiliência)**: Sistema de análise temporal que permite navegar por trajetórias históricas e simular futuros possíveis.
*   **Analysis Workspace (Contextos Observacionais)**: Espaço dedicado para Contexto Narrativo, Discursivo, Trajectory e Recomendações, separado das operações 3D.
*   **Simulação de Cenários**: Ferramentas integradas para projetar **Estabilidade**, **Fragmentação** de habitat e **Desmatamento**, com visualização imediata de impacto (Ghost Mode).
*   **Trajectory Impact Profile (AI)**: O sistema analisa todo o histórico de mudanças do território para gerar perfis de impacto detalhados, identificando tendências de degradação estrutural e desvios de resiliência.
*   **Visualização Híbrida**: Renderização avançada de Nuvens de Pontos (LIDAR/CSV) e Terrenos Sólidos (Mesh/OBJ) com mapeamento espacial automático de dados científicos ("Spatial Mapping").
*   **Assistência Cognitiva**: Integração nativa com LLMs locais (Qwen/Ollama) para interpretação hermenêutica dos dados quantitativos.

## Comece Aqui (Getting Started)

*   📖 **[Manual do Usuário](docs/MANUAL_DO_USUARIO.md)**: Guia completo de operação (Teclas, Menus, Ferramentas).
*   🧪 **[Exemplos Práticos](docs/EXEMPLOS_PRATICOS.md)**: Tutorial passo-a-passo com dados de exemplo.

## Documentação Fundamental

Para entender a filosofia, a governança dos modelos e a arquitetura de conhecimento que rege este projeto, consulte:

*   **[Fundação Científica (Scientific Foundation)](docs/SISTERSTRATA_SCIENTIFIC_FOUNDATION.md)**: O documento "constitucional/white paper" que define as leis físicas, o contrato de pipeline e a estruturação dos domínios.
*   **[Funcionalidades Científicas](docs/SCIENTIFIC_FUNCTIONALITIES.md)**: Resumo das funcionalidades, aplicações e limitações com links detalhados.
*   **[Pedosfera (SCORPAN/SiBCS)](docs/SCIENTIFIC_PEDOSFERA.md)**: Detalhamento científico do domínio de solo.
*   **[Hidrologia](docs/SCIENTIFIC_HIDROLOGIA.md)**: Detalhamento científico do domínio hidrológico.
*   **[Vegetação](docs/SCIENTIFIC_VEGETACAO.md)**: Detalhamento científico do domínio de vegetação.
*   **[Quarta Dimensão (Resiliência)](docs/SCIENTIFIC_QUARTA_DIMENSAO.md)**: Detalhamento científico de trajetória e coerência.
*   **[Manual do Usuário](docs/MANUAL_DO_USUARIO.md)**: Guia operacional das funcionalidades da ferramenta (Simulação, Visualização 3D, Timeline).
*   **[Análise de Trajetória de Patch (DDD)](docs/DDD_PatchTrajectory_Analysis.md)**: Definição do modelo multi-estado para patches.
*   **[Arquitetura Vetorial de Hipóteses](docs/Arquitetura_Vetorial_Hipoteses.md)**: Formalização matemática do espaço vetorial de parâmetros.
*   **[Fourth Dimension (Resilience)](docs/DDD_FourthDimensionSystem.md)**: O modelo de resiliência e coerência temporal.
*   **[Contexto de Observação Narrativa](docs/NARRATIVE_OBSERVATION_CONTEXT_CONTRACTS.md)**: Sistema observacional para registrar interpretações discursivas do território.
*   **[Architecture & DDD](docs/projeto_app_ddd_v10.md)**: Organização do código e princípios de design.
*   **[STRATA Governance Model v0.1](STRATA_Governance_Model_v0_1.md)**: Governança estrutural e política normativa de causalidade/ADR.
*   **[ADR Index (Governança Arquitetural)](adr/README.md)**: Registro canônico e histórico de decisões arquiteturais.
*   **Architecture Decision Index (gerado)**: `reports/architecture/ArchitectureDecisionIndex.latest.json` e `.md` via `python3 scripts/build_adr_catalog.py`.

## Requisitos de Sistema

### Hardware
- **GPU**: Compatível com Vulkan 1.2+ (NVIDIA RTX 20 series+ ou AMD Radeon 5000+ recomendadas para 14b LLM).
- **RAM**: 16GB mínimo (64GB recomendados para rodar Qwen 14b simultaneamente).

### Software & Bibliotecas
- **Compilador**: GCC 10+ ou Clang 10+ (suporte completo a C++20).
- **Vulkan SDK**: Incluindo `glslc` para compilação de shaders.
- **SDL2**: Gestão de janelas e inputs.
- **CMake**: Versão 3.20 ou superior.
- **AI (Opcional)**: [Ollama](https://ollama.com/) para assistência cognitiva local.
    - Modelo recomendado: `qwen2.5:14b` (ou `qwen2.5:7b` para máquinas com menos VRAM).

## Build e Instalação

### 1. Instalar Dependências (Linux)

**Fedora 43:**
```bash
sudo dnf install cmake gcc-c++ SDL2-devel vulkan-loader-devel shaderc
```
> [!NOTE]
> Para detalhes específicos de configuração de GPU NVIDIA e Drivers no Fedora 43, consulte as **[Notas de Build do Fedora](BUILD_NOTES_FEDORA43_SisterSTRATA.md)**.

**Ubuntu 22.04+ / Debian:**
```bash
sudo apt update && sudo apt install cmake g++ libsdl2-dev libvulkan-dev shaderc
```

### 2. Compilação

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### 3. Execução

```bash
./bin/SisterSTRATA
```

### 4. Configuração da IA (Opcional)
Para habilitar os recursos de **Assistência Cognitiva** e **Análise de Impacto**:
1.  Instale o Ollama: `curl -fsSL https://ollama.com/install.sh | sh`
2.  Baixe o modelo: `ollama run qwen2.5:14b`
3.  O SisterSTRATA detectará o Ollama automaticamente ao iniciar.

---

Consulte `ARCHITECTURE.md` para detalhes sobre a estrutura do projeto e princípios de design.
