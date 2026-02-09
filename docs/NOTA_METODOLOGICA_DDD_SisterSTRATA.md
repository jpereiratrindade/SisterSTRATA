# Nota Metodológica
## Domain-Driven Design no SisterSTRATA
### Por que não é Documentation-Driven Development

Projeto: SisterSTRATA  
Contexto: Modelagem de Sistemas Socioecológicos  
Status: Documento Metodológico Canônico

---

## 1. Objetivo desta Nota

Esta nota metodológica tem como objetivo explicitar a abordagem de
desenvolvimento adotada no SisterSTRATA, esclarecendo:

- por que o projeto se insere na perspectiva de Domain-Driven Design (DDD);
- qual é o papel da documentação no processo;
- por que essa abordagem NÃO deve ser confundida com
  Documentation-Driven Development;
- como essa escolha está diretamente relacionada à natureza científica,
  socioecológica e inferencial do STRATA.

Este documento visa alinhar a equipe, colaboradores e futuros leitores
do projeto quanto aos fundamentos metodológicos adotados.

---

## 2. A Natureza do Problema Enfrentado pelo STRATA

O SisterSTRATA não é um sistema convencional de informação ou simulação.
Seu objetivo central é:

> Inferir estados, trajetórias e propriedades emergentes (ex.: resiliência)
> de sistemas socioecológicos a partir de evidências processuais,
> principalmente oriundas da literatura científica.

Isso implica lidar com:
- sistemas complexos;
- múltiplas dimensões (ecológica, produtiva, social);
- causalidade indireta e mediada;
- conhecimento incompleto, contextual e historicamente situado.

Nesses termos, o principal desafio do STRATA não é tecnológico,
mas conceitual e epistemológico.

---

## 3. Por que Domain-Driven Design (DDD)

O Domain-Driven Design é adotado no SisterSTRATA porque:

- o domínio do problema NÃO é trivial nem autoevidente;
- conceitos como “estado”, “trajetória” e “resiliência” não são dados,
  mas inferidos;
- é necessário construir uma Linguagem Ubíqua compartilhada
  entre pesquisadores e sistema computacional;
- o software deve EXPRESSAR o domínio, não apenas operacionalizá-lo.

No STRATA, o domínio central não é o sistema socioecológico em si,
mas o processo de inferência sobre esse sistema.

---

## 4. O Papel da Documentação no STRATA

A documentação no SisterSTRATA exerce um papel FUNDACIONAL,
não meramente descritivo.

Ela é utilizada para:
- cristalizar o entendimento coletivo do domínio;
- explicitar princípios epistemológicos;
- registrar invariantes conceituais;
- delimitar responsabilidades entre contextos;
- orientar a modelagem DDD.

Importante:
A documentação NÃO substitui o modelo de domínio.
Ela o torna explícito.

---

## 5. Por que isso NÃO é Documentation-Driven Development

Documentation-Driven Development (DocDD) pressupõe que:

- o documento é a autoridade final;
- o código implementa o documento;
- a evolução ocorre principalmente via revisão textual.

Essa NÃO é a abordagem do SisterSTRATA.

No STRATA:
- a autoridade é o domínio modelado;
- a documentação expressa o domínio, mas não o governa isoladamente;
- o modelo evolui de forma incremental, junto com o entendimento do domínio;
- documentos são revisados quando o domínio evolui, não o contrário.

Portanto, o STRATA não é “desenvolvido a partir de documentos”,
mas sim “desenvolvido a partir de um domínio explicitado em documentos”.

---

## 6. A Abordagem Adotada: DDD com Documentação Fundacional

A abordagem metodológica do SisterSTRATA pode ser descrita como:

> Domain-Driven Design orientado por documentação fundacional explícita.

Características centrais:
- DDD como estrutura principal;
- documentação como instrumento de alinhamento epistemológico;
- separação clara entre extração, inferência e interpretação;
- rejeição de ontologias rígidas precoces;
- ênfase em processos, trajetórias e relações causais.

Essa abordagem é particularmente adequada a sistemas científicos,
onde o domínio não preexiste ao sistema, mas é construído junto com ele.

---

## 7. Implicações Arquiteturais

Essa escolha metodológica implica que:

- Bounded Contexts são definidos por responsabilidades inferenciais,
  não por módulos técnicos;
- invariantes epistemológicas são tratadas como invariantes de domínio;
- serviços de domínio concentram a inteligência inferencial;
- entidades permanecem simples e rastreáveis;
- o sistema privilegia diagnósticos processuais,
  e não classificações normativas.

---

## 8. Considerações Finais

A adoção consciente de Domain-Driven Design no SisterSTRATA
não é uma escolha estilística, mas uma necessidade metodológica.

Ela permite:
- rigor conceitual;
- transparência epistemológica;
- escalabilidade analítica;
- automação progressiva sem perda de sentido científico.

Esta nota deve ser lida como documento de referência
para qualquer desenvolvimento, extensão ou interpretação
do SisterSTRATA.

---

## 9. Status do Documento

Tipo: Nota Metodológica  
Natureza: Fundacional  
Uso:
- Alinhamento da equipe
- Comunicação metodológica
- Proteção conceitual do projeto
- Referência para novos colaboradores
