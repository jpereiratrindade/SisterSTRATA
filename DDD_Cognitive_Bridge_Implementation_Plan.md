# DDD Cognitive Bridge: Implementation Plan v1.1

## 1. Contexto e Objetivos
Este plano descreve a integração do LLM Qwen na plataforma SisterSTRATA como um **Observador Interpretativo de Segundo Nível**. A integração segue os princípios de DDD e garante a separação entre o domínio bio-físico e os discursos narrativos.

## 2. Ajustes Arquiteturais (A partir do feedback do expert)

### A. ContextBundles (Pacotes Semânticos)
A IA não será chamada com dados ad-hoc. Toda chamada ao serviço de IA será mediada por um `ContextBundle`, que agrupa explicitamente os objetos de domínio (Narrativas, DSC, REC) que formam a base da interpretação.

### B. InterpretationSnapshots (Memória Epistemológica)
As interpretações da IA não são voláteis. Elas serão salvas como objetos `InterpretationSnapshot` no banco de dados, permitindo auditoria, rastreabilidade e comparação temporal de como a IA está "enxergando" os discursos.

### C. Refatoração para CognitiveAssistanceService
O serviço anterior (`InterpretationService`) será renomeado para `CognitiveAssistanceService`, operando através de **Modos de Interpretação** (ThemeAnalysis, DiscursiveDraft, etc.), evitando uma interface rígida.

---

## 3. Detalhamento Técnico

### Infraestrutura
- **LlmClient**: Cliente HTTP assíncrono para comunicação com Ollama (API local).
- **Endpoint**: Localhost:11434 (padrão).

### Aplicação (O Core da Ponte)
- `InterpretationMode`: Enum com os modos definidos no DDD.
- `CognitiveAssistanceService::interpret(ContextBundle, Mode)`: Método principal que formata o prompt usando o **Contrato Canônico** e despacha para o LLM.

### User Interface
- **Interpretation Modal**: Componente comum para exibir a resposta da IA.
- **Botões de Gatilho**: Adicionados aos painéis correspondentes (DSC, NOC, REC).

---

## 4. Plano de Ação Imediato

1. **[NEW]** Criar structs `ContextBundle` e `InterpretationSnapshot` no domínio.
2. **[NEW]** Implementar `CognitiveAssistanceService` com o **System Prompt Canônico**.
3. **[NEW]** Implementar `LlmClient` para comunicação básica.
4. **[UI]** Adicionar botão de "Análise de Temas" no painel de Narrativas como primeiro teste funcional.

---

## 5. Regras de Segurança (Inegociáveis)
- O prompt do sistema deve proibir explicitamente o uso de linguagem prescritiva ("você deve", "o ideal é").
- A IA nunca recebe estados bio-físicos brutos (apenas descrições textuais).
- O humano é sempre o validador final antes de qualquer salvamento.
