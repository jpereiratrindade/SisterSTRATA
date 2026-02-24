#pragma once
#include <map>
#include <string>

namespace strata::domain::energy {

class EnergyAllocationPolicy {
public:
    virtual ~EnergyAllocationPolicy() = default;
    
    // Contrato base para dividir a energia entre as demandas.
    virtual std::map<std::string, double> allocate(
        double available_energy,
        const std::map<std::string, double>& requests) = 0;
};

// V0.1 Equalitarian distribution policy
class EqualitarianPolicy : public EnergyAllocationPolicy {
public:
    std::map<std::string, double> allocate(
        double available_energy,
        const std::map<std::string, double>& requests) override;
};

} // namespace strata::domain::energy
