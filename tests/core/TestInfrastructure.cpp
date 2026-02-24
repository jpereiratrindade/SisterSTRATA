#include <iostream>
#include "core/domain/energy/EnergyPool.hpp"
#include "core/domain/energy/EnergyAllocationPolicy.hpp"
#include "core/domain/identity/IdentityNode.hpp"
#include "core/domain/seto/SoilMonitorNode.hpp"
#include "core/domain/infrastructure/InfrastructureOrchestrator.hpp"
#include "core/domain/simulation/EnvironmentController.hpp"

int main() {
    using namespace strata::domain;

    std::cout << "Starting Infrastructure DDD v0.1 Test...\n";

    // 1. Initialize Energy Pool (Capacity: 10000 Wh, Initial: 5000 Wh)
    energy::EnergyPool pool(10000.0, 5000.0);

    // 2. Policy
    energy::EqualitarianPolicy policy;

    // 3. Nodes
    // FocinhoTrack: 2.0 Wh per event, 5.0 Wh base consumption, let's say 20 events per animal/day
    identity::IdentityNode identity_node(20.0, 2.0, 5.0);

    // SETO: 10.0 Wh base consumption, max 40.0 Wh dynamic based on moisture
    seto::SoilMonitorNode soil_node(10.0, 40.0);

    // 4. Orchestrator
    infrastructure::InfrastructureOrchestrator orchestrator(
        pool,
        policy,
        identity_node,
        soil_node
    );

    // 5. Simulation Controller (365 days)
    simulation::EnvironmentController controller(365);
    
    // 6. Run & Export CSV
    std::string csv_path = "infrastructure_simulation_v01.csv";
    controller.run(orchestrator, csv_path);

    std::cout << "Done.\n";
    return 0;
}
