# Análise de Integração: Qwen2.5:7b no SisterSTRATA

## 1. Visão Geral
Trazer o **Qwen2.5:7b** para o SisterSTRATA não é apenas um "recurso de chat", mas uma evolução para o pilar de **Rastreabilidade de Resultados** (Seção 2.3 da Fundação Científica). O modelo atuaria como um **Intérprete Científico Semântico**, capaz de traduzir os estados derivados (pedogênese, hidrologia) em explicações causais para o usuário.

## 2. Casos de Uso Potenciais

### A. O Oráculo Científico (Sonda Semântica)
Ao clicar em uma célula do terreno, o usuário pode perguntar: *"Por que aqui foi classificado como Latossolo?"*.
O sistema enviaria para o Qwen os fatores SCORPAN daquela célula (ex: P=Granito, C=Tropical, R=Declividade Baixa, A=Antigo) e o modelo explicaria a lógica pedogenética por trás do resultado, citando o SiBCS.

### B. Síntese de Bacias Hidrográficas
O modelo pode ler os relatórios de drenagem gerados e criar uma síntese qualitativa: *"Esta bacia apresenta alto risco de erosão na região nordeste devido à combinação de solo argiloso e convergência de fluxo (TWI alto), recomendando-se atenção à cobertura vegetal."*

### C. Geração de Cenários por Linguagem Natural
Controle da simulação via texto: *"Transforme o clima em semi-árido e observe a resposta da vegetação em 50 anos"*. O modelo traduziria o comando para parâmetros internos da engine.

## 3. Análise Técnica de Viabilidade

| Fator | Impacto | Análise |
| :--- | :--- | :--- |
| **Hardware** | Alto | O Qwen2.5:7b requer ~5.5GB de VRAM (4-bit). Como a engine usa Vulkan intensamente, pode haver competição por recursos de GPU. |
| **Performance** | Médio | A inferência leva segundos. Deve ser executada em uma thread separada (Thread Pool) para não travar a renderização (60fps). |
| **Dependências** | Baixo | Recomendamos o uso da **API do Ollama** (localhost:11434). Exige apenas uma biblioteca minimalista de HTTP/JSON no C++. |

## 4. Arquitetura Proposta

### Camada de Infraestrutura (`src/infrastructure/ai`)
- **WebClient**: Handler para requisições assíncronas ao Ollama.
- **PromptManager**: Templates que injetam a "personalidade" do cientista do SisterSTRATA e as regras do SiBCS/SCORPAN.

### Camada de Domínio (`src/core/domain/ai`)
- **ScientificAssistant**: Interface que consome dados do `SoilGrid` e `HydroGrid` para contextualizar as respostas.

### Camada de UI (`src/ui`)
- **AIOraclePanel**: Janela flutuante (ImGui) com histórico de conversa e sugestões baseadas no contexto da seleção atual.

## 5. Próximos Passos Sugeridos
1. **PoC (Proof of Concept)**: Integrar uma biblioteca HTTP (ex: `cpp-httplib`) e testar um "Hello World" com o Ollama local.
2. **Contextualização**: Implementar o "System Prompt" que define o conhecimento acadêmico do modelo sobre solos e hidrologia.
3. **Draft de UI**: Criar o painel flutuante no ImGui para interação direta.

---
**Conclusão**: A integração é **altamente recomendada** para elevar o software de uma ferramenta de vizualização para uma plataforma de consultoria científica assistida.
