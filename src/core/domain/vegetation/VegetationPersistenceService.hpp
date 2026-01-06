#pragma once

#include "VegetationSystemOriginal.hpp"
#include "VegetationDeclarationService.hpp"
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

namespace Core::Domain::Vegetation {

/**
 * @brief Domain Service responsible for text-based serialization of vegetation scenarios.
 */
class VegetationPersistenceService {
public:
    /**
     * @brief Saves the vegetation system state to a text file.
     */
    static bool saveScenarios(const VegetationSystemOriginal& system, const std::string& filepath) {
        std::ofstream file(filepath);
        if (!file.is_open()) return false;

        for (const auto& scenario : system.getScenarios()) {
            file << "SCENARIO: " << scenario.getId() << "\n";
            for (const auto& comp : scenario.getComponents()) {
                file << "  COMPONENT: " << comp.getType().toString() << "\n";
                const auto& cond = comp.getConditions();
                if (cond.minSlope.has_value()) file << "    MIN_SLOPE: " << cond.minSlope.value() << "\n";
                if (cond.maxSlope.has_value()) file << "    MAX_SLOPE: " << cond.maxSlope.value() << "\n";
                if (cond.maxDistanceToDrainage.has_value()) file << "    MAX_DIST_DRAINAGE: " << cond.maxDistanceToDrainage.value() << "\n";
                file << "  COMPONENT_END\n";
            }
            file << "SCENARIO_END\n";
        }
        return true;
    }

    /**
     * @brief Loads vegetation scenarios from a text file into the system.
     */
    static bool loadScenarios(VegetationSystemOriginal& system, const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) return false;

        system.clear();
        std::string line;
        std::string currentScenarioId;
        
        while (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string key;
            ss >> key;

            if (key == "SCENARIO:") {
                ss >> currentScenarioId;
            } else if (key == "COMPONENT:") {
                std::string typeStr;
                ss >> typeStr;
                
                VegetationCode code = VegetationCode::Campestre;
                if (typeStr == "FlorestalNatural") code = VegetationCode::FlorestalNatural;
                else if (typeStr == "Agua") code = VegetationCode::Agua;

                ReliefCondition cond;
                while (std::getline(file, line) && line.find("COMPONENT_END") == std::string::npos) {
                    std::stringstream css(line);
                    std::string ckey;
                    css >> ckey;
                    if (ckey == "MIN_SLOPE:") { float v; css >> v; cond.minSlope = v; }
                    else if (ckey == "MAX_SLOPE:") { float v; css >> v; cond.maxSlope = v; }
                    else if (ckey == "MAX_DIST_DRAINAGE:") { float v; css >> v; cond.maxDistanceToDrainage = v; }
                }
                
                system.addHypothesis(VegetationOriginal(HypothesisID(currentScenarioId), VegetationType(code), cond));
            }
        }
        return true;
    }
};

} // namespace Core::Domain::Vegetation
