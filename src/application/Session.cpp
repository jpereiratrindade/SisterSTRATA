#include "application/Session.hpp"
#include "core/domain/energy/EnergyPool.hpp"
#include "core/domain/energy/EnergyAllocationPolicy.hpp"
#include "core/domain/identity/IdentityNode.hpp"
#include "core/domain/soil/SoilMonitorNode.hpp"
#include "core/domain/infrastructure/InfrastructureOrchestrator.hpp"
#include "core/domain/simulation/EnvironmentController.hpp"
#include <algorithm>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace {

std::string nowTimestampToken() {
    const auto now = std::chrono::system_clock::now();
    const std::time_t tt = std::chrono::system_clock::to_time_t(now);
    std::tm tm {};
#if defined(_WIN32)
    localtime_s(&tm, &tt);
#else
    localtime_r(&tt, &tm);
#endif
    std::ostringstream out;
    out << std::put_time(&tm, "%Y%m%d_%H%M%S");
    return out.str();
}

std::string nowHumanTimestamp() {
    const auto now = std::chrono::system_clock::now();
    const std::time_t tt = std::chrono::system_clock::to_time_t(now);
    std::tm tm {};
#if defined(_WIN32)
    localtime_s(&tm, &tt);
#else
    localtime_r(&tt, &tm);
#endif
    std::ostringstream out;
    out << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return out.str();
}

const char* operationalStateLabel(strata::domain::infrastructure::OperationalState state) {
    using strata::domain::infrastructure::OperationalState;
    switch (state) {
        case OperationalState::Full: return "Full";
        case OperationalState::Reduced: return "Reduced";
        case OperationalState::Survival: return "Survival";
        case OperationalState::Suspended: return "Suspended";
        case OperationalState::Recovering: return "Recovering";
        default: return "Unknown";
    }
}

} // namespace

namespace Application {

std::string Session::runInfrastructureResilienceSimulation(int days) {
    InfrastructureEvaluationConfig config;
    config.days = days;
    return runInfrastructureResilienceSimulation(config);
}

std::string Session::runInfrastructureResilienceSimulation(const InfrastructureEvaluationConfig& config) {
    using namespace strata::domain;

    int days = config.days <= 0 ? 365 : config.days;
    const int ftNodeCount = std::max(1, config.ftNodeCount);
    const double ftHardwareCostUsd = std::max(0.0, config.ftHardwareCostUsd);

    try {
        const std::filesystem::path reportDir = projectRoot_ / "reports" / "infrastructure";
        std::filesystem::create_directories(reportDir);

        const std::string runToken = nowTimestampToken();
        const std::filesystem::path latestCsvPath = reportDir / "InfrastructureSimulation.latest.csv";
        const std::filesystem::path stampedCsvPath = reportDir / ("InfrastructureSimulation_" + runToken + ".csv");
        const std::filesystem::path latestJsonPath = reportDir / "InfrastructureResilience.latest.json";
        const std::filesystem::path stampedJsonPath = reportDir / ("InfrastructureResilience_" + runToken + ".json");

        energy::EnergyPool pool(10000.0, 5000.0);
        energy::EqualitarianPolicy policy;
        identity::IdentityNode identityNode(config.identityEventsPerAnimalPerDay, config.identityProfile);
        soil::SoilMonitorNode soilNode(config.soilProfile);
        infrastructure::InfrastructureOrchestrator orchestrator(pool, policy, identityNode, soilNode);
        simulation::EnvironmentController controller(days);

        controller.run(orchestrator, latestCsvPath.string());
        if (std::filesystem::exists(latestCsvPath)) {
            std::filesystem::copy_file(
                latestCsvPath,
                stampedCsvPath,
                std::filesystem::copy_options::overwrite_existing
            );
        }

        const auto& finalPool = orchestrator.getEnergyPool();
        const auto& finalIdentity = orchestrator.getIdentityNode();
        const auto& finalSoil = orchestrator.getSoilNode();
        const auto& identityProfile = finalIdentity.energyProfile();
        const auto& soilProfile = finalSoil.energyProfile();
        const auto& identityReqBreakdown = finalIdentity.requestedBreakdown();
        const auto& identityConsBreakdown = finalIdentity.consumedBreakdown();
        const auto& soilReqBreakdown = finalSoil.requestedBreakdown();
        const auto& soilConsBreakdown = finalSoil.consumedBreakdown();

        nlohmann::json report = nlohmann::json::object();
        report["schemaVersion"] = "infrastructure_resilience_report.v0.1";
        report["generatedAt"] = nowHumanTimestamp();
        report["projectRoot"] = projectRoot_.string();
        report["trigger"] = config.trigger.empty() ? "analysis_workspace_manual_run" : config.trigger;
        report["runConfig"] = {
            {"days", days},
            {"poolCapacityWh", 10000.0},
            {"poolInitialWh", 5000.0},
            {"identity",
                {
                    {"eventsPerAnimalPerDay", config.identityEventsPerAnimalPerDay},
                    {"energyModel", "device_operational_profile_v0.1"},
                    {"energyPerEventWh", identityProfile.sensing_wh_per_event + identityProfile.processing_wh_per_event + identityProfile.communication_wh_per_event},
                    {"baseConsumptionWh", identityProfile.boot_wh_per_day + identityProfile.idle_wh_per_day},
                    {"profile",
                        {
                            {"bootWhPerDay", identityProfile.boot_wh_per_day},
                            {"idleWhPerDay", identityProfile.idle_wh_per_day},
                            {"sensingWhPerEvent", identityProfile.sensing_wh_per_event},
                            {"processingWhPerEvent", identityProfile.processing_wh_per_event},
                            {"communicationWhPerEvent", identityProfile.communication_wh_per_event}
                        }
                    }
                }
            },
            {"soil",
                {
                    {"energyModel", "device_operational_profile_v0.1"},
                    {"baseConsumptionWh", soilProfile.boot_wh_per_day + soilProfile.idle_wh_per_day + soilProfile.sensing_base_wh_per_day + soilProfile.communication_base_wh_per_day},
                    {"maxDynamicConsumptionWh", soilProfile.dynamic_measurement_max_wh},
                    {"profile",
                        {
                            {"bootWhPerDay", soilProfile.boot_wh_per_day},
                            {"idleWhPerDay", soilProfile.idle_wh_per_day},
                            {"sensingBaseWhPerDay", soilProfile.sensing_base_wh_per_day},
                            {"communicationBaseWhPerDay", soilProfile.communication_base_wh_per_day},
                            {"dynamicMeasurementMaxWh", soilProfile.dynamic_measurement_max_wh}
                        }
                    }
                }
            },
            {"ftHardware",
                {
                    {"nodeCount", ftNodeCount},
                    {"estimatedHardwareCostUSD", ftHardwareCostUsd},
                    {"components", config.ftComponentSelection}
                }
            }
        };
        report["artifacts"] = {
            {"csvLatest", latestCsvPath.string()},
            {"csvSnapshot", stampedCsvPath.string()},
            {"jsonLatest", latestJsonPath.string()},
            {"jsonSnapshot", stampedJsonPath.string()}
        };
        report["finalState"] = {
            {"poolStorageWh", finalPool.currentStorage()},
            {"identity",
                {
                    {"requestedWh", finalIdentity.requestedEnergy()},
                    {"allocatedWh", finalIdentity.allocatedEnergy()},
                    {"consumedWh", finalIdentity.consumedEnergy()},
                    {"state", operationalStateLabel(finalIdentity.currentState())},
                    {"reliabilityIndex", finalIdentity.reliabilityIndex()},
                    {"processedEvents", finalIdentity.processedEvents()},
                    {"totalDailyEvents", finalIdentity.totalDailyEvents()},
                    {"requestedBreakdownWh",
                        {
                            {"bootWh", identityReqBreakdown.boot_wh},
                            {"idleWh", identityReqBreakdown.idle_wh},
                            {"sensingWh", identityReqBreakdown.sensing_wh},
                            {"processingWh", identityReqBreakdown.processing_wh},
                            {"communicationWh", identityReqBreakdown.communication_wh},
                            {"totalWh", identityReqBreakdown.total_wh}
                        }
                    },
                    {"consumedBreakdownWh",
                        {
                            {"bootWh", identityConsBreakdown.boot_wh},
                            {"idleWh", identityConsBreakdown.idle_wh},
                            {"sensingWh", identityConsBreakdown.sensing_wh},
                            {"processingWh", identityConsBreakdown.processing_wh},
                            {"communicationWh", identityConsBreakdown.communication_wh},
                            {"totalWh", identityConsBreakdown.total_wh}
                        }
                    }
                }
            },
            {"soil",
                {
                    {"requestedWh", finalSoil.requestedEnergy()},
                    {"allocatedWh", finalSoil.allocatedEnergy()},
                    {"consumedWh", finalSoil.consumedEnergy()},
                    {"state", operationalStateLabel(finalSoil.currentState())},
                    {"reliabilityIndex", finalSoil.reliabilityIndex()},
                    {"requestedBreakdownWh",
                        {
                            {"bootWh", soilReqBreakdown.boot_wh},
                            {"idleWh", soilReqBreakdown.idle_wh},
                            {"sensingBaseWh", soilReqBreakdown.sensing_base_wh},
                            {"communicationWh", soilReqBreakdown.communication_wh},
                            {"sensingDynamicWh", soilReqBreakdown.sensing_dynamic_wh},
                            {"totalWh", soilReqBreakdown.total_wh}
                        }
                    },
                    {"consumedBreakdownWh",
                        {
                            {"bootWh", soilConsBreakdown.boot_wh},
                            {"idleWh", soilConsBreakdown.idle_wh},
                            {"sensingBaseWh", soilConsBreakdown.sensing_base_wh},
                            {"communicationWh", soilConsBreakdown.communication_wh},
                            {"sensingDynamicWh", soilConsBreakdown.sensing_dynamic_wh},
                            {"totalWh", soilConsBreakdown.total_wh}
                        }
                    }
                }
            }
        };

        {
            std::ofstream latestOut(latestJsonPath);
            if (latestOut.is_open()) {
                latestOut << report.dump(2);
            }
        }
        {
            std::ofstream stampedOut(stampedJsonPath);
            if (stampedOut.is_open()) {
                stampedOut << report.dump(2);
            }
        }

        return latestJsonPath.string();
    } catch (...) {
        return {};
    }
}

void Session_KeepAlive() {}

} // namespace Application
