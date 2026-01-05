#pragma once

#include "PatchTrajectory.hpp"
#include <string>
#include <sstream>
#include <functional>

namespace Core::Domain::FourthDimension::PatchTrajectory {

/**
 * @brief Domain Service to analyze trajectories and generate reports/summaries.
 * Follows Section 8 of DDD_PatchTrajectory_Analysis.
 */
class PatchTrajectoryService {
public:
    /**
     * @brief Generates a semantic summary of the trajectory for the LLM.
     * @param trajectory The trajectory to summarize.
     * @param nameResolver Optional function to convert category IDs to names.
     */
    static std::string generateLLMSummary(const PatchTrajectory& trajectory, 
                                          std::function<std::string(int)> nameResolver = nullptr) {
        std::stringstream ss;
        ss << "PATCH_TRAJECTORY_SUMMARY\n";
        ss << "- lifespan: " << (trajectory.getLifespan() > 5 ? "longo" : "curto") << " (" << trajectory.getLifespan() << " estados)\n";
        
        float trend = trajectory.getNetAreaTrend();
        ss << "- net_area_trend: " << (trend > 10.0f ? "ganho" : (trend < -10.0f ? "perda" : "estável")) << " (" << trend << ")\n";
        
        float stability = trajectory.getStructuralStabilityIndex();
        ss << "- structural_stability: " << (stability > 0.8f ? "alta" : "baixa") << " (index: " << stability << ")\n";
        
        float volatility = trajectory.getShapeVolatility();
        ss << "- shape_volatility: " << (volatility > 0.5f ? "alta" : "baixa") << "\n";

        // Adjacency Contrast (from the last state)
        if (!trajectory.getHistory().empty()) {
            const auto& last = trajectory.getHistory().back();
            ss << "- land_use_adjacency_contrast: ";
            if (last.adjacencyByClass.empty()) {
                ss << "desconhecido";
            } else {
                for (auto const& [cls, val] : last.adjacencyByClass) {
                    std::string clsName = nameResolver ? nameResolver(cls) : ("Classe " + std::to_string(cls));
                    ss << "[" << clsName << ": " << val << "%] ";
                }
            }
            ss << "\n";
        }
        
        // Semantic classification (Section 6)
        ss << "- dominant_trajectory_type: " << classify(trajectory) << "\n";
        
        return ss.str();
    }

private:
    static std::string classify(const PatchTrajectory& trajectory) {
        float trend = trajectory.getNetAreaTrend();
        float stability = trajectory.getStructuralStabilityIndex();
        float volatility = trajectory.getShapeVolatility();
        
        if (volatility > 0.8f) return "Pulsátil";
        if (trend < -20.0f) return "Erosiva";
        if (trend > 20.0f) return "Reorganizativa";
        if (stability > 0.9f && std::abs(trend) < 5.0f) return "Estável";
        if (volatility > 0.4f && stability < 0.6f) return "Fragmentante";
        
        return "Transitória";
    }
};

} // namespace Core::Domain::FourthDimension::PatchTrajectory
