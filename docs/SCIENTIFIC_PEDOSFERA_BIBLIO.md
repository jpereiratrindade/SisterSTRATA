# Pedosfera (SCORPAN / SiBCS) — Revisão Bibliográfica e Fundamentação Epistemológica

Este documento reúne a revisão bibliográfica essencial e a fundamentação
epistemológica do domínio Pedosfera no projeto SisterSTRATA.

Ele complementa o documento canônico `SCIENTIFIC_PEDOSFERA.txt`, fornecendo o
embasamento teórico, histórico e crítico que sustenta as escolhas científicas
realizadas no sistema.

Este texto não descreve implementação, parâmetros computacionais ou arquitetura
de software. Seu papel é exclusivamente científico-conceitual.

---

## 1. O solo como corpo natural e memória ecológica

A pedologia moderna concebe o solo como um corpo natural dinâmico, formado ao
longo do tempo pela interação entre fatores físicos, químicos, biológicos e
antrópicos. Clássicos da área reconhecem o solo não apenas como suporte para a
vegetação, mas como registro material da história ambiental de uma paisagem.

Essa concepção é fundamental para sistemas que buscam analisar trajetórias
espaciais e temporais, uma vez que o solo atua como mediador de processos
ecossistêmicos e condicionante estrutural de estados futuros.

---

## 2. A formulação de Jenny e a gênese das funções pedológicas

Hans Jenny (1941) formalizou uma das mais influentes concepções da pedologia ao
expressar o solo como função de fatores ambientais:

S = f(cl, o, r, p, t)

onde clima, organismos, relevo, material de origem e tempo interagem na
formação do solo.

A importância dessa formulação reside menos em sua capacidade preditiva direta
e mais em seu valor heurístico, ao organizar o pensamento pedológico em torno
de fatores reconhecíveis e operacionalizáveis.

---

## 3. Do modelo CLORPT ao SCORPAN

McBratney, Mendonça-Santos e Minasny (2003) expandiram a formulação original de
Jenny ao incorporar explicitamente o fator espacial, resultando no modelo
SCORPAN:

S = f(S, C, O, R, P, A, N)

A inclusão de S (solo pré-existente) e N (posição espacial) reflete avanços
metodológicos associados à pedometria e ao uso de geotecnologias.

O SCORPAN consolidou-se como um arcabouço funcional amplamente utilizado em
mapeamento digital de solos, especialmente em contextos de grandes extensões
territoriais.

---

## 4. SCORPAN como estrutura heurística, não modelo mecanístico

Diversos autores destacam que o SCORPAN não descreve explicitamente os processos
pedogenéticos (adição, perda, transformação e translocação), mas organiza
relações funcionais entre fatores e propriedades do solo.

Assim, seu uso adequado requer cautela epistemológica: o modelo orienta
inferências plausíveis, mas não estabelece causalidade direta nem substitui
estudos de campo ou modelos mecanísticos detalhados.

No SisterSTRATA, essa limitação é assumida de forma explícita e incorporada
como princípio de design científico.

---

## 5. O papel do tempo na pedologia e em modelos computacionais

Na pedologia clássica, o tempo representa tanto duração de processos quanto
estágio de desenvolvimento do solo. Em modelos computacionais, entretanto,
o tempo frequentemente assume formas proxy ou relativas.

O SisterSTRATA adota o tempo como índice comparativo entre estados e cenários,
evitando interpretações cronológicas absolutas que não podem ser sustentadas
pelos dados disponíveis.

Essa escolha dialoga com abordagens contemporâneas em ecologia de sistemas e
modelagem espacial, nas quais o foco recai sobre trajetórias e padrões emergentes.

---

## 6. Sistemas de classificação de solos e a SiBCS

Sistemas de classificação, como a Soil Taxonomy (USDA) ou o Sistema Brasileiro
de Classificação de Solos (SiBCS), organizam a diversidade pedológica em classes
discretas a partir de critérios diagnósticos.

A SiBCS, desenvolvida no contexto brasileiro, reflete condições tropicais e
subtropicais e constitui referência institucional para estudos e aplicações no
país.

Entretanto, como todo sistema classificatório, a SiBCS não descreve processos
causais, mas fornece categorias interpretativas que auxiliam na comunicação,
comparação e análise de solos.

---

## 7. Limitações conhecidas da classificação pedológica

A literatura reconhece limitações recorrentes em sistemas classificatórios,
especialmente em:

- ambientes campestres e ecótonos
- mosaicos pedológicos de alta variabilidade espacial
- transições graduais entre classes
- escalas incompatíveis entre observação e classificação

Essas limitações reforçam a necessidade de tratar classes pedológicas como
representações operacionais, e não como descrições exaustivas da realidade.

---

## 8. Integração da pedosfera em sistemas socioecológicos

Estudos em ecologia de paisagens e sistemas socioecológicos enfatizam que o
solo atua como condicionante estrutural, interagindo com hidrologia,
vegetação e uso da terra.

No SisterSTRATA, essa integração é realizada de forma não normativa, mantendo
a pedosfera como base biofísica que informa, mas não determina, trajetórias
ou decisões.

---

## 9. Implicações epistemológicas para o SisterSTRATA

A revisão bibliográfica apresentada sustenta as seguintes posições adotadas
no projeto:

- uso de funções pedológicas como heurísticas controladas
- separação entre estado inferido e solo real observado
- rejeição de causalidade forte não suportada por dados
- explicitação de limites e incertezas como parte do modelo

Esses princípios visam garantir honestidade científica e evitar extrapolações
indevidas dos resultados do sistema.

---

## 10. Referências bibliográficas essenciais

- Jenny, H. (1941). *Factors of Soil Formation*. McGraw-Hill.
- McBratney, A. B., Mendonça-Santos, M. L., & Minasny, B. (2003).
  On digital soil mapping. *Geoderma*, 117, 3–52.
- EMBRAPA (2018). *Sistema Brasileiro de Classificação de Solos* (5ª ed.).
- Hartemink, A. E. (2015). The use of soil classification in soil science.
- Minasny, B., & McBratney, A. B. (2016). Digital soil mapping: A brief history.

Outras referências específicas podem ser adicionadas conforme a ampliação
do escopo científico do projeto.
