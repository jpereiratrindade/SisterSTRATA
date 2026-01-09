===============================================================================
DDD – VEGETATIONSYSTEMORIGINAL
STRATA – CORE DOMAIN
===============================================================================

VERSÃO: 1.0
STATUS: ATIVO (ARQUITETURAL)
ESCOPO: Core Domain
NATUREZA: Declarativo (Hipótese Inicial)
DEPENDÊNCIAS: Relevo, Drenagem, Solo
EXCLUSÕES: Uso da Terra, Resiliência, Dinâmica Vegetacional

-------------------------------------------------------------------------------
1. CONTEXTO E MOTIVAÇÃO
-------------------------------------------------------------------------------

O STRATA é concebido como um laboratório de trajetórias eco-geomorfológicas.
Durante a evolução do projeto, identificou-se que a simulação direta de uso da
terra e resiliência introduzia decisões ontológicas prematuras e normativas.

Para garantir clareza conceitual, separação de domínios e honestidade científica,
foi introduzido o sistema VegetationSystemOriginal.

Este sistema NÃO simula vegetação.
Ele declara explicitamente hipóteses ecológicas iniciais plausíveis,
servindo como referência estrutural para análises futuras.

-------------------------------------------------------------------------------
2. DEFINIÇÃO DO BOUNDED CONTEXT
-------------------------------------------------------------------------------

Nome do Contexto:
VegetationSystemOriginal

Tipo:
Bounded Context Declarativo

Responsabilidade Central:
Definir e armazenar a vegetação potencial inicial do território,
com base em critérios biofísicos explícitos, sem dinâmica temporal.

Este contexto pertence ao Core Domain.

-------------------------------------------------------------------------------
3. PRINCÍPIOS INEGOCIÁVEIS
-------------------------------------------------------------------------------

1) VegetationSystemOriginal NÃO é um sistema de simulação.
2) VegetationSystemOriginal NÃO evolui no tempo.
3) VegetationSystemOriginal NÃO responde a perturbações.
4) VegetationSystemOriginal NÃO avalia resiliência.
5) VegetationSystemOriginal NÃO define uso da terra.
6) Toda vegetação definida aqui é uma HIPÓTESE explícita.
7) Nenhuma regra normativa deve ser inferida a partir deste sistema.

Violação de qualquer princípio acima caracteriza erro arquitetural.

-------------------------------------------------------------------------------
4. CONCEITO CENTRAL (LINGUAGEM UBÍQUA)
-------------------------------------------------------------------------------

Vegetação Potencial:
Campo de possibilidades ecológicas plausíveis de ocupação vegetal,
condicionado por relevo, drenagem e solo,
anterior ou marginal à antropização dominante.

Hipótese Inicial:
Configuração declarada pelo usuário, versionável e explícita,
utilizada como referência, nunca como resultado.

-------------------------------------------------------------------------------
5. ENTIDADES E VALUE OBJECTS
-------------------------------------------------------------------------------

5.1 Entidade Principal

Entity: VegetationOriginal
Identidade: VegetationOriginalID

Responsabilidade:
Representar a vegetação potencial declarada para o território.

Atributos:
- VegetationType
- SpatialDistribution
- HypothesisID
- CreationMetadata

A entidade é IMUTÁVEL após criada.

-------------------------------------------------------------------------------

5.2 Value Objects

Value Object: VegetationType
Valores permitidos (v1):
- Campestre
- FlorestalNatural

Observação:
Novos tipos podem ser adicionados futuramente,
desde que mantenham caráter declarativo.

---

Value Object: ReliefCondition
Representa critérios derivados do relevo, tais como:
- posição topográfica (topo, encosta, fundo de vale)
- declividade
- curvatura
- proximidade de drenagem

IMPORTANTE:
ReliefCondition NÃO é regra causal.
É apenas critério de plausibilidade espacial.

---

Value Object: HypothesisID
Identificador explícito da hipótese ecológica declarada.
Permite versionamento e comparação entre cenários.

-------------------------------------------------------------------------------
6. AGREGADO
-------------------------------------------------------------------------------

Aggregate Root:
VegetationSystemOriginal

Contém:
- VegetationOriginal
- Conjunto de critérios declarativos
- Metadados da hipótese

Invariantes do agregado:
- Nenhuma lógica de simulação
- Nenhuma atualização temporal
- Nenhuma dependência de UI ou infraestrutura

-------------------------------------------------------------------------------
7. DOMAIN SERVICES
-------------------------------------------------------------------------------

Service: VegetationDeclarationService

Responsabilidade:
- Criar instâncias de VegetationOriginal
- Validar consistência interna da declaração
- Garantir que critérios são explícitos e completos

O serviço NÃO:
- avalia impacto
- avalia estabilidade
- avalia resiliência
- executa alocação dinâmica

-------------------------------------------------------------------------------
8. RELAÇÃO COM OUTROS CONTEXTOS
-------------------------------------------------------------------------------

Dependências Permitidas (unidirecionais):
- Relevo (Slope, Curvature, Position)
- Drenagem (Rede, Distância, Conectividade)
- Solo (Domínio, não classe determinística)

Dependências PROIBIDAS:
- Uso da Terra
- Resiliência
- Clima dinâmico
- Processos biológicos

-------------------------------------------------------------------------------
9. INTERFACE COM OUTROS SISTEMAS
-------------------------------------------------------------------------------

VegetationSystemOriginal pode ser consumido por:
- futuros sistemas de vegetação dinâmica
- sistemas de uso da terra
- módulos de análise e avaliação
- análise de resiliência

Sempre como REFERÊNCIA.
Nunca como regra ou alvo.

-------------------------------------------------------------------------------
10. DIRETRIZES PARA IMPLEMENTAÇÃO
-------------------------------------------------------------------------------

1) Implementar como estrutura simples e explícita.
2) Priorizar clareza semântica sobre otimização.
3) Garantir imutabilidade após criação.
4) Documentar toda hipótese criada.
5) Versionar hipóteses sempre que alteradas.
6) Nunca acoplar lógica de avaliação ou decisão.

-------------------------------------------------------------------------------
11. NOTA FINAL À EQUIPE
-------------------------------------------------------------------------------

VegetationSystemOriginal é uma escolha metodológica consciente.

Ele existe para:
- explicitar pressupostos
- organizar o espaço de possibilidades
- evitar simulações prematuras

Hipóteses declaradas são preferíveis
a processos mal compreendidos.

Qualquer tentativa de “fazer o sistema agir”
deve ocorrer em outro contexto.

===============================================================================
FIM DO DOCUMENTO
===============================================================================
