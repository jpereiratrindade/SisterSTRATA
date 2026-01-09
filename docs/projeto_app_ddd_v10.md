===============================================================================
DOMAIN-DRIVEN DESIGN (DDD)
Plataforma Científica de Processamento de Dados e Visualização 3D
===============================================================================

VERSÃO: 1.0
STATUS: Documento Fundacional
LINGUAGEM: C++ (17/20)
PLATAFORMA: Desktop (Linux / Windows)
FOCO: Processamento Científico de Alto Desempenho

===============================================================================
1. VISÃO GERAL DO PRODUTO
===============================================================================

Este software é uma aplicação desktop científica, desenvolvida em C++,
com foco em processamento intensivo de dados e visualização avançada
em ambiente 3D de mundo finito.

O sistema foi concebido para:

- Execução multithread em CPU
- Uso opcional de GPU
- Separação rigorosa entre ciência, visualização e infraestrutura
- Desenvolvimento colaborativo por múltiplos desenvolvedores

O produto oferece dois espaços funcionais complementares:

1) Espaço de Processamento Científico de Dados
2) Espaço de Processamento Científico em Ambiente 3D Interativo

Ambos compartilham o mesmo núcleo científico.

===============================================================================
2. DESIGN ESTRATÉGICO
===============================================================================

-------------------------------------------------------------------------------
2.1 CORE DOMAIN
-------------------------------------------------------------------------------

Nome do Domínio Central:
Scientific Processing Core

Responsabilidade:
- Representar dados científicos
- Executar transformações e pipelines determinísticos
- Garantir consistência matemática, física ou ecológica
- Ser independente de UI, GPU e renderização

Este domínio representa o principal valor do software e deve ser
estritamente protegido de dependências externas.

-------------------------------------------------------------------------------
2.2 SUBDOMÍNIOS
-------------------------------------------------------------------------------

a) Subdomínio de Visualização 3D
   - Mundo finito e alta resolução
   - Navegação First Person Observer View (FPOV)
   - Renderização com Vulkan

b) Subdomínio de Interface Científica (UI)
   - Menus
   - Painéis
   - Abas
   - Gerenciamento de arquivos e sessões
   - Implementado com ImGui

c) Subdomínio de Infraestrutura
   - Multithreading
   - Agendamento de tarefas
   - Abstração CPU/GPU
   - Sistema de arquivos
   - SDL2 (janela e input)

-------------------------------------------------------------------------------
2.3 LINGUAGEM UBÍQUA
-------------------------------------------------------------------------------

Termo                     Significado
--------------------------------------------------
Dataset                   Conjunto de dados científicos
Pipeline                  Sequência de operações científicas
Workspace                 Contexto ativo do usuário
View                      Representação visual (2D ou 3D)
World                     Espaço 3D finito
Task                      Unidade mínima de execução
Session                   Estado global de trabalho
Frame                     Estado renderizado do mundo 3D

Todos os termos devem aparecer:
- No código
- Nos documentos
- Na interface do usuário

-------------------------------------------------------------------------------
2.4 BOUNDED CONTEXTS
-------------------------------------------------------------------------------

1) Scientific Core Context
   - Modelos científicos
   - Algoritmos
   - Pipelines
   - Sem dependência de UI ou GPU

2) Data Processing Workspace Context
   - Gerenciamento de arquivos
   - Painel lateral esquerdo (datasets abertos)
   - Painel principal com abas

3) 3D World Context
   - Mundo finito
   - Câmera FPOV
   - Sincronização com dados científicos

4) UI Context
   - ImGui
   - Menus
   - Layouts
   - Eventos do usuário

5) Infrastructure Context
   - Threads
   - Job system
   - GPU abstraction
   - File system

-------------------------------------------------------------------------------
2.5 MAPA DE CONTEXTO (DESCRITIVO)
-------------------------------------------------------------------------------

Scientific Core
   ↑ fornece dados para
Data Workspace  ←→  3D World
        ↑                ↑
        └──── UI Context ┘
               ↑
        Infrastructure Context

O Core Domain nunca depende de UI, Vulkan, SDL2 ou GPU.

-------------------------------------------------------------------------------
2.6 ANTI-CORRUPTION LAYER (ACL)
-------------------------------------------------------------------------------

- Adaptadores entre dados científicos e buffers gráficos
- Tradutores entre modelos científicos e widgets ImGui
- Abstrações entre CPU e GPU

===============================================================================
3. DESIGN TÁTICO
===============================================================================

-------------------------------------------------------------------------------
3.1 ENTIDADES
-------------------------------------------------------------------------------

- Dataset
  Identidade: DatasetID
  Representa dados científicos carregados

- Workspace
  Identidade: WorkspaceID
  Representa o estado ativo do usuário

- World
  Identidade: WorldID
  Representa um mundo 3D finito

-------------------------------------------------------------------------------
3.2 VALUE OBJECTS
-------------------------------------------------------------------------------

- FilePath
- Resolution
- Vector3
- CameraPose
- ThreadConfig
- GPUCapabilities

São imutáveis e validados no construtor.

-------------------------------------------------------------------------------
3.3 AGREGADOS
-------------------------------------------------------------------------------

Aggregate Root:
Workspace

Contém:
- Datasets
- Pipelines ativos
- Views abertas
- Estado da sessão

Todas as operações relevantes partem do Workspace.

-------------------------------------------------------------------------------
3.4 DOMAIN SERVICES
-------------------------------------------------------------------------------

- PipelineExecutionService
- DataTransformationService
- WorldSimulationService

===============================================================================
4. ARQUITETURA
===============================================================================

-------------------------------------------------------------------------------
4.1 ESTILO ARQUITETURAL
-------------------------------------------------------------------------------

Arquitetura Hexagonal (Ports & Adapters)

- Domínio isolado
- UI, Vulkan, SDL2 e GPU como adaptadores
- Facilita testes, manutenção e evolução

-------------------------------------------------------------------------------
4.2 CAMADA DE APLICAÇÃO
-------------------------------------------------------------------------------

Responsável por:
- Orquestrar casos de uso
- Reagir a comandos do usuário
- Coordenar serviços de domínio

===============================================================================
5. ORGANIZAÇÃO DO CÓDIGO E DESENVOLVIMENTO COLABORATIVO
===============================================================================

-------------------------------------------------------------------------------
5.1 PRINCÍPIOS DE ORGANIZAÇÃO
-------------------------------------------------------------------------------

1. Cada Bounded Context possui diretório próprio
2. Cada contexto tem API pública clara
3. Não são permitidas dependências circulares
4. O Core Domain compila isoladamente
5. Dependências seguem o fluxo:
   UI → Application → Domain
6. Cada desenvolvedor pode atuar em um contexto específico

-------------------------------------------------------------------------------
5.2 ESTRUTURA DE PASTAS DO PROJETO
-------------------------------------------------------------------------------

project_root/
│
├── CMakeLists.txt
├── README.md
├── docs/
│   └── DDD_Plataforma_Cientifica.txt
│
├── external/
│   ├── imgui/
│   ├── glm/
│   └── stb/
│
├── src/
│   ├── core/
│   │   ├── CMakeLists.txt
│   │   ├── domain/
│   │   ├── services/
│   │   └── value_objects/
│   │
│   ├── application/
│   │   ├── CMakeLists.txt
│   │   ├── use_cases/
│   │   └── ports/
│   │
│   ├── infrastructure/
│   │   ├── CMakeLists.txt
│   │   ├── filesystem/
│   │   ├── threading/
│   │   └── gpu/
│   │
│   ├── ui/
│   │   ├── CMakeLists.txt
│   │   ├── menus/
│   │   ├── panels/
│   │   └── docking/
│   │
│   ├── world3d/
│   │   ├── CMakeLists.txt
│   │   ├── world/
│   │   ├── camera/
│   │   └── rendering/
│   │
│   └── main.cpp
│
└── tests/
    ├── core/
    └── application/

-------------------------------------------------------------------------------
5.3 CONTRATO DE COLABORAÇÃO
-------------------------------------------------------------------------------

- Alterações no Core Domain exigem revisão técnica
- UI e 3D não podem alterar regras científicas
- Infraestrutura não define comportamento de domínio
- Comunicação entre contextos ocorre via interfaces

===============================================================================
6. CONSIDERAÇÕES FINAIS
===============================================================================

Este documento define não apenas o modelo conceitual do sistema,
mas também sua organização técnica e social.

O DDD atua como:
- Guia arquitetural
- Contrato de equipe
- Base para evolução sustentável do software

===============================================================================
FIM DO DOCUMENTO
===============================================================================

