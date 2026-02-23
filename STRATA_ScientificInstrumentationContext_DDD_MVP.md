# STRATA — ScientificInstrumentationContext
## Domain-Driven Design (DDD) — MVP v1.0
### SoilElectricalLab + HardwareSimulation Integration

---

# 1. Contexto Estratégico

O **ScientificInstrumentationContext** é um macro-domínio do STRATA responsável por:

> Projetar, validar e simular instrumentos científicos a partir de exigências ontológicas do modelo territorial.

Ele formaliza a relação entre:
- Simulação estrutural
- Observabilidade elétrica
- Viabilidade instrumental

Este contexto possui dois eixos fundamentais:

- Epistemic Axis → Detectabilidade científica
- Instrument Axis → Simulação de hardware

---

# 2. Bounded Contexts Internos

ScientificInstrumentationContext é composto por:

```
ScientificInstrumentationContext
    ├── SoilElectricalLabContext
    └── HardwareSimulationContext
```

---

# 3. SoilElectricalLabContext

## 3.1 Responsabilidade

Determinar se transições estruturais do solo são detectáveis eletricamente e derivar requisitos instrumentais mínimos.

## 3.2 Linguagem Ubíqua

- StructuralState
- StructuralTransition
- ElectricalResponse
- ResistivityShift
- DetectabilityThreshold
- SignalToNoiseRatio
- DetectabilityRequirement

---

## 3.3 Entidades

### SoilState (Aggregate Root)
- id
- bulkDensity
- volumetricWaterContent
- porosity
- timestamp

### StructuralTransition
- fromState
- toState
- transitionType (Compaction | MoistureChange | Combined)

### ElectricalState
- resistivity
- conductivity
- derivedFrom (SoilState)
- frequency

### DetectionExperiment (Aggregate Root)
- baselineElectricalState
- perturbedElectricalState
- noiseModel
- detectionThreshold

### DetectabilityReport
- deltaResistivity
- signalToNoiseRatio
- isDetectable
- thresholdUsed

---

## 3.4 Value Objects

- MoistureLevel
- DensityValue
- PorosityValue
- FrequencyBand
- NoiseLevel
- ADCResolution
- PhaseAngle

---

## 3.5 Modelo Físico MVP

Lei de Archie simplificada:

ρ = a * ρw * φ^(-m) * Sw^(-n)

Onde:
- ρ = resistividade
- φ = porosidade
- Sw = saturação aproximada
- a, m, n = parâmetros explícitos

---

## 3.6 Output Principal

### DetectabilityRequirement

- minimumDeltaResistivity
- requiredSNR
- recommendedFrequencyBand

Este objeto é o contrato com o HardwareSimulationContext.

---

# 4. HardwareSimulationContext

## 4.1 Responsabilidade

Simular comportamento instrumental e avaliar se os requisitos de detectabilidade são alcançáveis com determinada configuração de hardware.

Este contexto NÃO define requisitos.
Ele apenas testa viabilidade.

---

## 4.2 Linguagem Ubíqua

- InstrumentConfiguration
- ADCModel
- CurrentSourceModel
- ElectrodeModel
- ThermalDrift
- QuantizationNoise
- HardwareFeasibility

---

## 4.3 Entidades

### InstrumentConfiguration (Aggregate Root)
- adcResolutionBits
- samplingRate
- currentAmplitude
- electrodeSpacing
- cableLength
- noiseFloor

### NoiseModel
- gaussianNoiseLevel
- thermalDrift
- contactImpedanceVariation

### HardwareFeasibilityReport
- achievableSNR
- minimumDetectableDelta
- meetsRequirement (boolean)
- limitingFactor

---

## 4.4 Regras

- HardwareSimulation recebe apenas DetectabilityRequirement.
- Não pode alterar requisitos.
- Deve produzir relatório explícito de limitação instrumental.

---

# 5. Fluxo Integrado

1. STRATA executa simulação estrutural.
2. SoilElectricalLab deriva ElectricalState.
3. DetectionExperiment calcula detectabilidade.
4. DetectabilityRequirement é gerado.
5. HardwareSimulation testa configurações instrumentais.
6. HardwareFeasibilityReport é produzido.
7. Resultado integra relatórios de infraestrutura do STRATA.

---

# 6. Eventos de Domínio

- StructuralTransitionDetected
- DetectabilityRequirementGenerated
- HardwareConfigurationTested
- HardwareLimitationIdentified
- InstrumentSpecificationValidated

---

# 7. Artefatos Gerados

## SoilElectricalObservability.latest.json

{
  "transitionType": "Compaction_10%",
  "deltaResistivity": 2.4,
  "snrRequired": 3.0,
  "recommendedFrequencyBand": "5-20Hz"
}

## HardwareFeasibility.latest.json

{
  "adcResolutionBits": 18,
  "achievableSNR": 3.4,
  "meetsRequirement": true,
  "limitingFactor": "thermal_noise"
}

---

# 8. Integração no Workspace STRATA

Novo menu:

Observabilidade
    ├── SoilElectricalLab
    └── HardwareSimulation

Infraestrutura passa a incorporar:

- Electrical Observability Index
- Instrumental Feasibility Score

---

# 9. Critérios de Conclusão do MVP

O MVP é considerado funcional quando:

- Três cenários estruturais são simuláveis
- Detectabilidade é quantificável
- Requisitos instrumentais mínimos são deriváveis
- Pelo menos duas configurações instrumentais podem ser testadas
- Relatórios JSON são gerados de forma reprodutível

---

# 10. Relação com SETO

SETO físico só pode ser projetado após:

DetectabilityRequirement + HardwareFeasibilityReport aprovados.

Ordem obrigatória:

STRATA → Requisito → Simulação Instrumental → Projeto Físico

Nunca o inverso.

---

# 11. Diretriz Ontológica

ScientificInstrumentationContext formaliza:

"O que pode ser observado em um sistema territorial depende tanto da dinâmica estrutural quanto da capacidade instrumental."

STRATA deixa de ser apenas simulador de sistema.

Passa a ser plataforma de design científico de instrumentos.

---

# Versão
v1.0 — MVP Core Detectability + Instrument Simulation
