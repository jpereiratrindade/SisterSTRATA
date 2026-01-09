# Manual do Usuário - SisterSTRATA v1.9.3

Este manual descreve as funcionalidades operacionais da plataforma SisterSTRATA.

## 1. Navegação e Visualização 3D
O ambiente principal é uma viewport 3D de alta performance baseada em Vulkan.

*   **Câmera**:
    *   `Botão Direito + Arrastar`: Rotacionar a câmera (Orbit).
    *   `Botão Meio + Arrastar`: Mover a câmera (Pan).
    *   `Scroll`: Zoom In/Out.
*   **Modos de Visualização**:
    *   **Nuvem de Pontos (Point Cloud)**: Visualização padrão para dados brutos (CSV/XYZ).
    *   **Malha (Mesh)**: Visualização de superfície sólida para terrenos gerados (OBJ).
    *   **Spatial Mapping**: O sistema colore automaticamente terrenos complexos com base nos dados de classificação, mesmo se a resolução da malha for diferente da simulação.

## 2. Painel "Fourth Dimension" (Timeline & Resiliência)
Este é o centro de controle para análise temporal e simulações.

### 2.1. Trajetórias (Timeline)
*   **Captura de Estado**: Permite "fotografar" o estado atual do território para criar um novo ponto na linha do tempo.
*   **Navegação**: Clique em qualquer fatia (TimeSlice) na lista para visualizar seus metadados.
*   **Ghost Mode**: Ao selecionar um estado passado e clicar em `View (Ghost)`, o sistema projeta visualmente aquele estado sobre o terreno 3D (ex: mostrando como era a floresta há 10 anos), sem alterar os dados atuais. As áreas de desmatamento aparecem em **Marrom (Solo)** e a floresta em **Verde**.

### 2.2. Ferramentas de Simulação
Disponíveis diretamente no painel:
*   **Simulate: Stability**: Gera uma projeção de estabilidade estrutural.
*   **Simulate: Fragmentation**: Simula a fragmentação progressiva de habitats.
*   **Simulate: Deforestation**: Cria um cenário de perda massiva de cobertura vegetal (-1 Soil), útil para análise de impacto e visualização de contraste. Use o "Ghost Mode" para ver o resultado 3D.

## 3. Assistência Cognitiva (AI)
O SisterSTRATA integra modelos de linguagem locais (Ollama/Qwen) para análise hermenêutica.

*   **Contexto Discursivo**: Analisa métricas quantitativas (Coerência, Volatilidade) e gera relatórios qualitativos.
*   **Trajectory Impact Profile**: Gera perfis de impacto detalhados analisando toda a trajetória de mudanças do território.

## 4. Análise Hidrológica e de Solo
*   **Drenagem**: Ferramenta para visualizar acumulação de fluxo e redes de drenagem sobre o terreno.
*   **Análise de Patches**: Identificação e contagem de manchas de vegetação para métricas de ecologia de paisagem.

## 5. Dicas de Uso
*   Se o terreno parecer "Verde Sólido" ao usar o Ghost Mode, verifique se está no modo correto. A versão v1.9.3 corrige isso automaticamente usando Mapeamento Espacial e terrenos de demonstração "Showcase" (com relevo).
