# Manual do Usuário - SisterSTRATA v1.9.3

O **SisterSTRATA** é uma plataforma científica de alta performance para análise, simulação e visualização de dados ecológicos complexos. Este manual serve como referência completa para todas as suas funcionalidades.

---

## 1. Instalação e Requisitos
O SisterSTRATA exige hardware moderno para processamento 3D e inteligência artificial local.

### Requisitos Mínimos
*   **SO**: Linux (Fedora 43 recomendado).
*   **Gráficos**: GPU compatível com Vulkan 1.2+ (NVIDIA ou AMD).
*   **Dependências**: CMake, SDL2, Vulkan SDK e um compilador C++20.

### Como Instalar
As instruções detalhadas de compilação e instalação de pacotes encontram-se no arquivo **[README.md](../README.md)** na raiz do projeto. Para usuários de Fedora 43, consulte o guia especializado: **[Notas de Build (Fedora 43)](../BUILD_NOTES_FEDORA43_SisterSTRATA.md)**.

### Configuração da IA (Ollama)
Para usar as funcionalidades de análise discursiva e hermenêutica, é necessário ter o **Ollama** instalado e rodando o modelo `qwen2.5` (7b ou 14b). Sem isso, as ferramentas de IA permanecerão desativadas ou em modo de simulação limitada.

### Modo CPU (No-GPU)
Quando não há GPU Vulkan disponível, é possível iniciar o SisterSTRATA no modo CPU (visualização 2D simplificada).

```bash
./bin/SisterSTRATA --no-gpu
```

Atalhos no modo CPU:
*   **W / A / S / D**: Navegação do plano.
*   **Setas**: Navegação do plano.
*   **Scroll (Roda)**: Zoom.
*   **TAB**: Alternar vista (XY/XZ/YZ).
*   **F**: Ajustar conteúdo à tela.

---

## 2. Controles e Navegação 3D

O ambiente principal utiliza uma câmera híbrida (Orbit/Free-Look) para explorar terrenos e nuvens de pontos.

### Teclado
*   **W / S**: Mover para frente / trás.
*   **A / D**: Mover para esquerda / direita (Strafe).
*   **Q / E**: Mover para baixo / cima (Elevação).
*   **SHIFT Esquerdo**: Correr (Aumentar velocidade de movimento).

### Mouse
*   **Botão Esquerdo + Arrastar**: **Orbitar** (Girar ao redor do ponto de interesse). Ideal para examinar um objeto ou terreno específico.
*   **Botão Direito + Arrastar**: **Free Look** (Olhar livremente). Estilo FPS, ideal para "voar" pelo cenário.
*   **Scroll (Roda)**: Zoom In / Zoom Out.

---

## 3. Configurações de Visualização (Tools > Settings)

O painel de configurações permite ajustar a renderização em tempo real.

### Iluminação (Lighting)
*   **Light Direction**: Controla a direção do "Sol" virtual (X, Y, Z). Altere para visualizar sombras e relevo.
*   **Light Color**: Cor da luz solar (RGB).
*   **Ambient Strength**: Intensidade da luz ambiente (0.0 a 1.0). Aumente para clarear sombras escuras.

### Nuvens de Pontos (Point Clouds / Basins)
Controles específicos para visualização de dados CSV/XYZ e Drenagem.
*   **Point Size**: Tamanho dos pontos (1.0 a 20.0). Aumente para tornar os dados mais visíveis.
*   **Color Mode**:
    *   *Source (CSV)*: Usa as cores definidas no arquivo de dados.
    *   *Single Color*: Aplica uma cor uniforme escolhida pelo usuário a todos os pontos.

### Performance
*   **VSync**: Sincronização vertical para evitar "tearing" na tela.
*   **Max FPS**: Limitador de taxa de quadros (0 = Ilimitado). Útil para economizar energia em laptops.
*   **Camera Speed**: Ajuste fino da velocidade base de movimento.

---

## 4. Gestão de Arquivos e Formatos

Acesse via **File > Open** ou atalhos na interface.

### Formatos Suportados
*   **Projetos (.strata)**: Salva/Carrega todo o estado da sessão (trajetórias, configurações).
*   **Malhas 3D (.obj)**: Terrenos sólidos e modelos 3D. O SisterSTRATA aplica coloração automática baseada em altura se o OBJ não tiver texturas.
*   **Nuvens de Pontos (.csv, .xyz, .txt)**:
    *   *Formato Padrão*: X, Y, Z, R, G, B
    *   *Detecção Automática*: O sistema detecta coordenadas grandes (UTM) e centraliza automaticamente o modelo para evitar erros de precisão (Jitter).
    *   *Raster Grids*: CSVs com cabeçalho `# Origin:` são tratados como grades de solo.

---

## 5. Análise Temporal (Painel "Fourth Dimension")

Este painel gerencia a dimensão temporal e a resiliência do território.
> **Fundamentação Científica**: [Quarta Dimensão (Resiliência)](SCIENTIFIC_QUARTA_DIMENSAO.md)

### Linha do Tempo (Trajectory)
*   **Capture State**: Registra o estado atual da simulação como um novo ponto na linha do tempo (T1, T2, etc.).
*   **Lista de Fatias**: Clique em qualquer item (TimeSlice) para ver seus metadados.
*   **View (Ghost Mode)**: Ao selecionar um estado passado e clicar neste botão, o sistema projeta visualmente aquele estado sobre o terreno 3D atual.
    *   *Visualização*: O modo Ghost exibe as mudanças de cobertura vegetal. Áreas desmatadas aparecem em **Marrom (Solo)** e áreas íntegras em **Verde**.
    *   *Sair*: Clique em "Exit Ghost Mode" para restaurar a visualização normal.

### Ferramentas de Simulação (Simulation Tools)
As ferramentas de simulação geram cenários hipotéticos baseados em modelos científicos:
> **Saiba mais**: [Funcionalidades Científicas](SCIENTIFIC_FUNCTIONALITIES.md)
*   **Simulate: Stability**: Calcula índices de estabilidade estrutural baseados em coerência espacial.
*   **Simulate: Fragmentation**: Simula a fragmentação progressiva de habitats e desconexão de patches.
*   **Simulate: Deforestation**: Gera um cenário de impacto massivo (-1 Soil). Útil para testar a resiliência do sistema e visualização de contraste.

---

## 6. Assistência Cognitiva e IA (Analysis Workspace)

O SisterSTRATA integra modelos de linguagem locais (Ollama/Qwen) para atuar como um "Copiloto Científico".
> **Manual Técnico**: [Integração Qwen](TECHNICAL_MANUAL_QWEN_INTEGRATION.md) | [Arquitetura Vetorial](Arquitetura_Vetorial_Hipoteses.md)

### Acesso aos contextos observacionais
A entrada canônica dos contextos é:
*   **View > Analysis Workspace**

Dentro do Workspace:
*   **Contexto Narrativo**:
    *   `Workspace Synthesis`
    *   `Narrative Observation Context` (abas: `Observation Log`, `Epistemic Memory`, `Context Graph`)
*   **Contexto Discursivo**:
    *   abas `Ingestion`, `Registered History`, `Epistemic Memory`
*   **Trajectory**:
    *   abas `Trajectory`, `Snapshots`, `Epistemic Memory`
    *   seção auxiliar `Ações 3D` (Fourth Dimension / Patch Analysis), quando disponível
*   **Recomendações**:
    *   `Strategic Global Synthesis` (abas `Strategic Audit`, `Synthesis Memory`)

*   **Ingestão de Sistema Discursivo (Aba "Ingestion")**:
    *   **Ask Qwen to Propose System**: Sintetiza um rascunho estruturado a partir dos **Sistemas Discursivos registrados** (ingest). Ideal para consolidar problemas, ações, mecanismos e efeitos já declarados.
    *   **Evaluate Logical Coherence**: Analisa o sistema discursivo *já cadastrado* (lista abaixo do formulário) em busca de falhas lógicas (ex: Problema sem Ação correspondente, Mecanismo sem Efeito). Gera um relatório de consistência interna.
*   **Trajectory Impact Profile**: Analisa todo o histórico de mudanças (Trajetória Completa) para gerar um relatório de impacto ambiental.
    > **Detalhes do Modelo**: [Impact Profile](SCIENTIFIC_TRAJECTORY_IMPACT_PROFILE.md)
*   **Configuração**: O sistema detecta automaticamente o modelo mais forte disponível (ex: `qwen2.5:14b`).

### Escopo da análise por seleção de registros
Para evitar análises amplas demais, os painéis contextuais (dentro do Analysis Workspace) permitem restringir o escopo enviado ao Qwen:
*   **Discursive**: marque itens na coluna `Sel` e mantenha `Use selected records only`.
*   **Narrative**: marque itens na coluna `Sel` e mantenha `Use selected observations only`.
*   **Recommendation**: marque itens na coluna `Sel` e mantenha `Use selected snapshots only`.

Se o modo de seleção estiver ativo e nenhum item for marcado, a análise não é executada e o sistema informa seleção vazia.

### Exportação de avaliações LLM em Markdown
No histórico de interpretações (aba de memória de cada contexto no Workspace):
*   **Export Selected .md**: exporta apenas snapshots marcados na tabela.
*   **Export Visible .md**: exporta todos os snapshots atualmente visíveis no contexto.

O arquivo `.md` inclui metadados de rastreabilidade (`snapshotId`, `createdAt`, `intent`, `sourceBundleId`, `promptVersion`) e o conteúdo da avaliação.

---

## 7. Ferramentas Científicas Específicas

### Hidrologia (Analyze Drainage)
> **Base Científica**: [Hidrologia](SCIENTIFIC_HIDROLOGIA.md)
*   Calcula o fluxo de acumulação de água baseado na topografia.
*   Exibe estatísticas como acumulação máxima e contagem de células de "rio".
*   Visualiza a rede de drenagem sobre o terreno.

### Análise de Patches (Patch Analysis)
> **Base Científica**: [Vegetação](SCIENTIFIC_VEGETACAO.md) | [Pedosfera](SCIENTIFIC_PEDOSFERA.md)
*   Identifica manchas (patches) de vegetação conectada.
*   Calcula métricas de paisagem: Área Total, Área Média, Índice de Forma.
*   Permite selecionar ("picar") um patch específico no 3D para ver seus dados individuais.
