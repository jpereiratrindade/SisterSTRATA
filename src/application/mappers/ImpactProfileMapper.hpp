#pragma once

#include "observational/impact_profile/domain/entities/TrajectoryImpactProfile.hpp"
#include <sstream>
#include <iomanip>

namespace Application::Mappers {

using namespace SisterSTRATA::Observational::ImpactProfile::Domain;

class ImpactProfileMapper {
public:
    static std::string toNaturalLanguage(const TrajectoryImpactProfile& profile) {
        std::stringstream ss;
        
        const auto& ref = profile.getReference();
        const auto& sl = profile.getStructuralDeviation();
        const auto& tp = profile.getTemporalPattern();

        ss << "ANÁLISE DE IMPACTO DE TRAJETÓRIA\n";
        ss << "--------------------------------\n";
        ss << "Referencial: " << ref.description << " [" << ref.referenceId << "]\n";
        ss << "Tipo de Comparação: " << referenceTypeToString(ref.type) << "\n\n";

        ss << "DESVIO ESTRUTURAL (Observado vs Referência):\n";
        ss << " - Tag Semântica: " << sl.semanticTag << "\n";
        ss << " - Delta de Fragmentação: " << std::fixed << std::setprecision(2) << sl.fragmentationIndexDelta 
           << " (Positivo indica maior fragmentação)\n";
        ss << " - Delta de Coerência Espacial: " << sl.spatialCoherenceDelta 
           << " (Positivo indica maior agrupamento)\n";
        ss << " - Diferença de Área: " << sl.areaTrendDelta << " unidades quadradas\n\n";

        ss << "DINÂMICA TEMPORAL:\n";
        ss << " - Tendência: " << deviationTrendToString(tp.trend) << "\n";
        ss << " - Descrição: " << tp.description << "\n";
        
        return ss.str();
    }

private:
    static std::string referenceTypeToString(ReferenceType type) {
        switch(type) {
            case ReferenceType::Historical: return "Histórico (Auto-referência temporal)";
            case ReferenceType::Simulated: return "Simulado (Contra-factual)";
            case ReferenceType::Theoretical: return "Teórico (Ideal)";
            case ReferenceType::ControlGroup: return "Grupo de Controle";
            default: return "Desconhecido";
        }
    }

    static std::string deviationTrendToString(DeviationTrend trend) {
        switch(trend) {
            case DeviationTrend::Convergent: return "Convergente (Retorno ao padrão)";
            case DeviationTrend::Divergent: return "Divergente (Afastamento contínuo)";
            case DeviationTrend::Parallel: return "Paralelo (Desvio constante)";
            case DeviationTrend::Erratic: return "Errático (Sem padrão claro)";
            default: return "Indeterminado";
        }
    }
};

} // namespace Application::Mappers
