# Plano de Integração: Cognitive Assistance Context (CAC) - Qwen

Este documento consolida a estratégia técnica e epistemológica para a integração do modelo Qwen como assistente cognitivo no SisterSTRATA, visando a interpretação de trajetórias de resiliência.

## 1. Visão Geral e Arquitetura

O **CAC (Cognitive Assistance Context)** atua como uma camada de auxílio interpretativo, desacoplada do Core Domain. Ele traduz métricas quantitativas de resiliência em narrativas hermenêuticas.

### 1.1 Arquitetura Ports & Adapters
Para garantir a pureza do domínio e flexibilidade de infraestrutura:
- **Port**: `Application::Ports::ILLMService` (Interface de abstração).
- **Adapter**: `Infrastructure::LLM::OllamaAdapter` (Comunicação via REST API com motor local).
- **UI**: Integrado ao `TimelinePanel`, permitindo análise sob demanda de fatias temporais (`TimeSlices`).

---

## 2. Fundação Científica: Coherence Mapping ($S$)

A âncora do assistente cognitivo é o **Serviço de Coerência**, que mede a estabilidade entre dois estados ($A, B$):

### 2.1 Dimensões de Similaridade
$I(i) = w_{type}S_{type} + w_{struct}S_{struct} + w_{edge}S_{edge}$

1.  **Similaridade de Tipo ($S_{type}$)**: Proximidade semântica entre coberturas (ex: Campestre $\leftrightarrow$ Floresta = 0.5).
2.  **Similaridade de Estrutura ($S_{struct}$)**: Divergência de Jensen-Shannon (JS) entre histogramas locais de vizinhança.
3.  **Similaridade de Bordas ($S_{edge}$)**: Preservação da densidade de bordas locais (fragmentação).

### 2.2 Propriedades Críticas
- **Simetria**: $Coherence(A \to B) = Coherence(B \to A)$.
- **Independência de Prioridade**: Opera sobre o estado fenomenológico resolvido, ignorando a ordem de declaração.
- **Baseline Nulo**: Validação contra mapas de ruído aleatório para distinguir estrutura de coincidência estatística.

---

## 3. Contrato Epistemológico (System Prompt v0.1)

O Qwen opera sob um regime de **Hermenêutica Controlada**:

### 3.1 Identidade e Postura
- **Observador Secundário**: Observa descrições de observações, mantendo distância ontológica do fenômeno.
- **Linguagem Condicional**: Uso obrigatório de termos como *"sugere que"*, *"pode indicar"*, *"compatível com a hipótese de"*.
- **Não-Autoritativo**: O assistente não executa ações nem define verdades científicas; ele "enriquece a dúvida".

### 3.2 Regras de Output
1.  **Ancoragem Numérica**: Toda afirmação deve citar a métrica bruta (ex: *"Intensidade 0.620 sugere..."*).
2.  **Separação Ontológica**: Distinção clara entre **Fato do Sistema** e **Interpretação Cognitiva**.
3.  **Sinais Fracos**: Captura de padrões emergentes que a métrica pura pode não evidenciar visualmente.

---

## 4. Estratégia de Implementação (Fases)

1.  **Fase 1 (Atual)**: Implementação de Ports e Mock Adapters para validação de UI.
2.  **Fase 2**: Implementação do `OllamaAdapter` real e conexão com motor local.
3.  **Fase 3**: Hardening do prompt e ajuste de hiperparâmetros ($w_{type}, w_{struct}, w_{edge}$).

---

> [!IMPORTANT]
> A verdade científica reside no STRATA. O CAC é uma ferramenta de auxílio à cognição do pesquisador, não um oráculo decisório.
