# SISTERSTRATA: FUNDAÇÃO CIENTÍFICA E ARQUITETURA DE CONHECIMENTO

**Versão do Documento:** 1.3
**Data:** 09 de Janeiro de 2026
**Status:** Consolidado

---

## 1. VISÃO E PROPÓSITO

O SisterSTRATA transcende a definição de um "software de terreno". Ele é projetado como um **instrumento científico digital** para a simulação de ecossistemas complexos e dinâmicos.

Diferente de *engines* de jogos, onde o objetivo final é a plausibilidade visual ("parece real"), o objetivo do SisterSTRATA é a **consistência causal** ("funciona como o real"). Se o solo é vermelho, não é por uma escolha artística de textura, mas porque a simulação geoquímica indicou a presença de hematita derivada de um material de origem específico sob um clima tropical.

### O Princípio da Integridade do Modelo
O valor do SisterSTRATA reside na confiança de que cada pixel na tela é o resultado de uma cadeia causal rastreável de hipóteses científicas.

---

## 2. GOVERNANÇA DOS MODELOS CIENTÍFICOS

Para manter a integridade científica enquanto o software cresce em complexidade, adotamos os seguintes pilares de governança:

### 2.1. Versionamento de Hipóteses
Modelos científicos não são estáticos; eles evoluem. O sistema deve permitir saber qual "versão da verdade" gerou um resultado.
*   **Exemplo:** A simulação de erosão hídrica pode usar o modelo *Sister-Hydro-v1.2* (baseado em fluxo simples) ou *Sister-Hydro-v2.0* (baseado em equações de Saint-Venant).
*   **Requisito:** Cada dataset gerado deve carregar metadados indicando quais modelos e versões foram utilizados.

### 2.2. Validação de Parâmetros
O software não deve permitir configurações fisicamente impossíveis sem um alerta explícito.
*   **Limites Rígidos (Hard Constraints):** Parâmetros que violam leis da física (ex: porosidade do solo > 100%, massa negativa).
*   **Limites Ecológicos (Soft Warning):** Parâmetros improváveis na natureza (ex: floresta tropical com 0mm de chuva), que geram alertas de "Cenário Extremo".

### 2.3. Rastreabilidade de Resultados
Todo artefato visual ou numérico deve ser explicável. O usuário deve ser capaz de clicar em um ponto do terreno e perguntar "Por que isto é assim?" e receber a árvore causal (ex: "É um Latossolo porque P=Granito, C=Tropical, T=Antigo").

---

## 3. O CONTRATO DE PIPELINE (INPUT-STATE-OUTPUT)

Para evitar a mistura entre "o que eu quero", "o que acontece" e "o que eu vejo", estabelecemos um contrato estrito de fluxo de dados.

### 3.1. Configurável (INPUT / CAUSA)
Elemento sob controle do usuário ou definidos pelo cenário inicial. Estas são as variáveis independentes.
*   **Clima:** Precipitação, Temperatura, Vento.
*   **Geologia:** Material de Origem (Litologia inicial), Tectônica (elevação base).
*   **Tempo:** Duração da simulação, passo de tempo ($dt$).
*   **Intervenção:** Adubação, queimada induzida, inserção de espécies.

### 3.2. Derivado (STATE / EFEITO)
Propriedades emergentes calculadas pelos modelos de simulação. O usuário *não edita* isso diretamente; ele edita os *inputs* para influenciar isso. Estas são as variáveis dependentes.
*   **Estado do Solo:** Umidade, compactação, conteúdo de Carbono Orgânico, pH.
*   **Estado Hidrológico:** Profundidade da lâmina d'água, velocidade do fluxo, taxa de infiltração.
*   **Estado da Vegetação:** Biomassa, índice de área foliar (LAI), estágio sucessional.

### 3.3. Observável (OUTPUT / VISUALIZAÇÃO)
Como o estado é apresentado ao observador humano. Alterar a visualização **nunca** deve alterar a simulação subjacente.
*   **Renderização:** Cor do pixel (Shader), texturas.
*   **Sondas (Probes):** Valores numéricos exibidos na UI.
*   **Gráficos e Relatórios:** Estatísticas agregadas.
*   **Exemplo Crítico:** Mudar o modo de visualização de "Cor Realista" para "Mapa de Calor de Erosão" muda apenas o *Observable*, sem tocar no *Derived*.

---

## 4. FUNDAÇÕES DOS DOMÍNIOS (CONSTITUIÇÃO DO MUNDO)

Abaixo estão definidos os modelos fundamentais atualmente integrados ao SisterSTRATA.

### 4.1. DOMÍNIO: PEDOSFERA (SOLO)
O solo no SisterSTRATA não é uma pintura estática, mas uma entidade dinâmica derivada do modelo **SCORPAN**.

#### A Equação Fundamental
$$ S = f(S, C, O, R, P, A, N) $$
Onde o Solo ($S$) é função de:
*   **S (Soil):** Propriedades pré-existentes (memória do sistema).
*   **C (Climate):** Clima (Precipitação, Temperatura).
*   **O (Organisms):** Vegetação e fauna.
*   **R (Relief):** Relevo (Declividade, Curvatura).
*   **P (Parent Material):** Geologia/Litologia.
*   **A (Age):** Tempo geológico/pedogenético.
*   **N (Space):** Posição espacial (interação com vizinhos).

#### Implementação Prática (SiBCS)
O sistema utiliza regras determinísticas para classificar o solo (Sistema Brasileiro de Classificação de Solos - SiBCS) com base nos atributos vetoriais derivado do SCORPAN.
*   *Exemplo:* $P=Rico$ + $C=Úmido$ + $R=Plano$ $\rightarrow$ Pedogênese Intensa $\rightarrow$ Latossolo.

### 4.2. DOMÍNIO: HIDROLOGIA
A água é o agente conector do sistema.

*   **Princípio da Conservação de Massa:** $V_{entrada} - V_{saida} = \Delta V_{armazenado}$.
*   **Fluxo Superficial:** A água move-se da célula de maior potencial gravitacional para a de menor, transportando sedimentos (Erosão/Deposição) e solutos.
*   **Infiltração:** A água que penetra no solo torna-se disponível para a vegetação ($O$) e altera as propriedades do solo ($S$).
*   **Drenagem D8:** Direção de fluxo baseada em declividade máxima (8 vizinhos).
*   **Acumulação de Fluxo:** Propagação topológica para obter área contribuinte por célula.
*   **Bacias e Relatórios:** Segmentação de bacias, TWI, densidade de drenagem e relatórios exportáveis.

### 4.3. DOMÍNIO: VEGETAÇÃO
A vegetação é modelada como um autômato celular complexo, sensível ao nicho ecológico.

*   **Vigor ($V_{cell}$):** Cada célula possui um estado de vigor vegetal que responde a disponibilidade de recursos (água, nutrientes do solo) e distúrbios.
*   **Sucessão Ecológica:** O sistema simula a transição de estados (ex: Solo Exposto $\rightarrow$ Herbáceas $\rightarrow$ Arbustivas $\rightarrow$ Floresta) baseado no tempo e nas condições ambientais.

### 4.4. DOMÍNIO: QUARTA DIMENSÃO (RESILIÊNCIA)
A dimensão temporal no SisterSTRATA não é apenas um log, mas uma trajetória interpretável.

*   **snapshots (TimeSlices):** Captura imutável de todo o estado do sistema em um instante $t$.
*   **Métrica de Coerência ($I_{coh}$):** Avaliação quantitativa da similaridade entre dois estados.
    $$ I_{coh}(x,y) = w_{type} S_{type} + w_{struct} S_{struct} + w_{edge} S_{edge} $$
    Onde $S_{struct}$ é derivado da Divergência de Jensen-Shannon ($D_{JS}$) entre histogramas espaciais:
    $$ S_{struct} = 1 - \frac{D_{JS}(Hist_A, Hist_B)}{\log 2} $$
*   **Trajetória de Patch (v1.1):** Estudo do ciclo de vida de manchas.
    *   **Trend de Área ($\Delta A$):** $A_{final} - A_{initial}$
    *   **Volatilidade de Forma ($V$):** Média das variações do Shape Index ($SI$):
        $$ V = \frac{1}{n} \sum |SI_i - SI_{i-1}| $$
    *   **Índice de Estabilidade Estrutural ($S$):** $S = \frac{1}{1 + V}$

---

### 4.5. DOMÍNIO: NARRATIVA E CONTEXTO OBSERVACIONAL
O SisterSTRATA reconhece que a paisagem não é qualificada apenas por fenômenos biofísicos, mas também por construtos sociais e discursivos.

*   **Separação Ontológica (Território vs. Mapa):** O sistema mantém uma distinção rigorosa entre o "Estado Biofísico" (derivado de simulação causal) e o "Estado Narrativo" (declarado por fontes externas como entrevistas ou relatórios).
*   **Natureza Observacional:** Diferente dos domínios Simulation-Bound (Solo, Hidro), o domínio Narrativo é *Read-Only* em relação ao mundo físico. Uma entrevista dizendo que "o solo é fértil" cria um *NarrativeState*, mas não altera quimicamente o *SoilState*.
*   **Ancoragem Espacial:** Narrativas são ancoradas ao território via `SpatialScope` (Pontos ou Manchas), permitindo a correlação visual entre o que é *dito* sobre o lugar e o que é *simulado* no lugar, sem contaminar a causalidade de um com o outro.

---

## 5. INTERFACE COGNITIVA E GOVERNANÇA DE DADOS

A Ponte Cognitiva (v2.0) integra Modelos de Linguagem de Grande Porte (LLMs) como uma camada de **Hermenêutica Científica**, permitindo o diálogo entre a simulação biofísica e o contexto humano.

### 5.1. Memória Epistêmica
Diferente das simulações físicas, que são determinísticas, a análise cognitiva produz artefatos interpretativos que devem ser preservados.
- **Snapshot Cognitivo:** Cada análise gerada pela IA é registrada como um artefato imutável contendo o prompt original, a resposta do modelo e o timestamp.
- **Socio-Ecological Traceability:** Permite rastrear como as interpretações do sistema mudaram ao longo do tempo ou conforme novos dados narrativos foram ingeridos.

### 5.2. O Contrato Moral (Integridade Epistemológica)
Para evitar o uso da IA como um "oráculo", a integração é regida por restrições invioláveis injetadas em cada interação:
- **Descritividade Estrita:** A IA deve descrever padrões, tensões e inconsistências. Está proibida de prescrever ações políticas ou técnicas sem base em algoritmos determinísticos.
- **Não-Causalidade Involuntária:** A IA não pode inferir causas bio-físicas (ex: "a seca causou a morte do patch") onde o simulador reporta apenas correlação temporal, a menos que o `ContextBundle` contenha essa prova explícita.
- **Hermenêutica Baseada em Fatos:** Cada afirmação da IA deve ser um reflexo dos dados derivados (Seção 4.4) ou observados (Seção 4.5).

---

*Este documento deve ser revisado sempre que uma nova camada científica for introduzida no SisterSTRATA.*
