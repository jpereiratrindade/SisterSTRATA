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

## 6. Assistência Cognitiva e IA (Painel "Discursive System")

O SisterSTRATA integra modelos de linguagem locais (Ollama/Qwen) para atuar como um "Copiloto Científico".
> **Manual Técnico**: [Integração Qwen](TECHNICAL_MANUAL_QWEN_INTEGRATION.md) | [Arquitetura Vetorial](Arquitetura_Vetorial_Hipoteses.md)

*   **Ingestão de Sistema Discursivo (Aba "Ingestion")**:
    *   **Ask Qwen to Propose System**: Preenche automaticamente um formulário de Sistema Discursivo lendo todas as observações narrativas registradas até o momento. Ideal para criar uma primeira versão estruturada a partir de anotações soltas.
    *   **Evaluate Logical Coherence**: Analisa o sistema discursivo *já cadastrado* (lista abaixo do formulário) em busca de falhas lógicas (ex: Problema sem Ação correspondente, Mecanismo sem Efeito). Gera um relatório de consistência interna.
*   **Trajectory Impact Profile**: Analisa todo o histórico de mudanças (Trajetória Completa) para gerar um relatório de impacto ambiental.
    > **Detalhes do Modelo**: [Impact Profile](SCIENTIFIC_TRAJECTORY_IMPACT_PROFILE.md)
*   **Configuração**: O sistema detecta automaticamente o modelo mais forte disponível (ex: `qwen2.5:14b`).

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
