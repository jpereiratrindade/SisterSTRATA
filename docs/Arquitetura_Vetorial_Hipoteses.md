# Arquitetura Vetorial de Hipóteses (SISTERSTRATA)

Esta arquitetura formaliza a transição de **Regras Declarativas** para um **Espaço Vetorial de Parâmetros**, permitindo a análise matemática do contraste entre estados ecológicos.

## 1. O Vetor de Hipótese (Input)
Cada hipótese $H$ definida pelo usuário é tratada como um vetor $V$ em um espaço de $n$ dimensões:

$$V_H = [p_1, p_2, p_3, \dots, p_n]$$

Onde, atualmente:
- $p_1$: Declividade Mínima (deg)
- $p_2$: Declividade Máxima (deg)
- $p_3$: Distância Máxima à Drenagem (m)

## 2. Vetor de Contraste (Deslocamento)
Ao transitar do Estado A para o Estado B, o sistema calcula o **Vetor de Deslocamento** ($\Delta V$):

$$\Delta V = V_{H,B} - V_{H,A}$$

A magnitude deste vetor $||\Delta V||$ representa a intensidade da intervenção experimental ou da mudança ambiental forçada no modelo.

## 3. Resposta do Sistema (Output Metrics)
O impacto do deslocamento $\Delta V$ é medido pelas métricas de trajetória do patch definidas no DDD:
- **Trend de Área ($\Delta A$)**: Resposta direta à expansão ou retração do nicho vetorial.
- **Estabilidade Estrutural ($S$)**: Resistência da forma do patch ao deslocamento paramétrico.
- **Volatilidade ($V$)**: Sensibilidade da mancha a pequenas variações nas coordenadas do vetor.

## 4. Epistemologia da Paisagem
Nesta arquitetura, a paisagem é vista como uma **Função de Transferência**:
$$Metricas(Patch) = f(\Delta V_{Hipótese})$$

Isso permite que o LLM (Qwen) realize interpretações táticas como:
> *"O incremento de $5m$ no parâmetro de distância ($p_3$) resultou em um ganho de área de $15\%$, indicando que este nicho é altamente sensível à proximidade hídrica nesta topografia específica."*

## 5. Roadmap de Implementação
- **v1.8.x**: Grounding Semântico (Nomes de Hipóteses no LLM). [Concluído]
- **v2.0.0**: Implementação da classe `EcologicalScenario` para agrupar múltiplos vetores e calcular magnitudes de contraste automaticamente.
