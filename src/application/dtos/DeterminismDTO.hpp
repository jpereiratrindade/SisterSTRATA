#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

namespace Application::DTO {

inline constexpr int kDeterministicPayloadSchemaVersion = 1;

enum class DeterminismTier {
    T0_NonDeterministic,
    T1_SeededDeterministic,
    T2_ReplayVerified
};

inline const char* determinismTierId(DeterminismTier tier) {
    switch (tier) {
        case DeterminismTier::T2_ReplayVerified:
            return "T2_ReplayVerified";
        case DeterminismTier::T1_SeededDeterministic:
            return "T1_SeededDeterministic";
        case DeterminismTier::T0_NonDeterministic:
        default:
            return "T0_NonDeterministic";
    }
}

struct DeterminismConfig {
    uint64_t seed{0};
    DeterminismTier tier{DeterminismTier::T0_NonDeterministic};
    std::vector<std::string> entropySources{};
};

struct DeterministicIdentityBreakdown {
    double bootWh{0.0};
    double idleWh{0.0};
    double sensingWh{0.0};
    double processingWh{0.0};
    double communicationWh{0.0};
    double totalWh{0.0};
};

struct DeterministicSoilBreakdown {
    double bootWh{0.0};
    double idleWh{0.0};
    double sensingBaseWh{0.0};
    double communicationWh{0.0};
    double sensingDynamicWh{0.0};
    double totalWh{0.0};
};

struct DeterministicIdentitySnapshot {
    double requestedWh{0.0};
    double allocatedWh{0.0};
    double consumedWh{0.0};
    int operationalStateCode{0};
    double reliabilityIndex{0.0};
    double processedEvents{0.0};
    double totalDailyEvents{0.0};
    DeterministicIdentityBreakdown requestedBreakdownWh{};
    DeterministicIdentityBreakdown consumedBreakdownWh{};
};

struct DeterministicSoilSnapshot {
    double requestedWh{0.0};
    double allocatedWh{0.0};
    double consumedWh{0.0};
    int operationalStateCode{0};
    double reliabilityIndex{0.0};
    DeterministicSoilBreakdown requestedBreakdownWh{};
    DeterministicSoilBreakdown consumedBreakdownWh{};
};

struct DeterministicStatePayload {
    int schemaVersion{kDeterministicPayloadSchemaVersion};
    double poolStorageWh{0.0};
    DeterministicIdentitySnapshot identity{};
    DeterministicSoilSnapshot soil{};
};

inline void to_json(nlohmann::json& j, const DeterminismConfig& value) {
    j = nlohmann::json{
        {"seed", value.seed},
        {"tier", determinismTierId(value.tier)},
        {"entropySources", value.entropySources}
    };
}

inline void to_json(nlohmann::json& j, const DeterministicIdentityBreakdown& value) {
    j = nlohmann::json{
        {"bootWh", value.bootWh},
        {"idleWh", value.idleWh},
        {"sensingWh", value.sensingWh},
        {"processingWh", value.processingWh},
        {"communicationWh", value.communicationWh},
        {"totalWh", value.totalWh}
    };
}

inline void to_json(nlohmann::json& j, const DeterministicSoilBreakdown& value) {
    j = nlohmann::json{
        {"bootWh", value.bootWh},
        {"idleWh", value.idleWh},
        {"sensingBaseWh", value.sensingBaseWh},
        {"communicationWh", value.communicationWh},
        {"sensingDynamicWh", value.sensingDynamicWh},
        {"totalWh", value.totalWh}
    };
}

inline void to_json(nlohmann::json& j, const DeterministicIdentitySnapshot& value) {
    j = nlohmann::json{
        {"requestedWh", value.requestedWh},
        {"allocatedWh", value.allocatedWh},
        {"consumedWh", value.consumedWh},
        {"operationalStateCode", value.operationalStateCode},
        {"reliabilityIndex", value.reliabilityIndex},
        {"processedEvents", value.processedEvents},
        {"totalDailyEvents", value.totalDailyEvents},
        {"requestedBreakdownWh", value.requestedBreakdownWh},
        {"consumedBreakdownWh", value.consumedBreakdownWh}
    };
}

inline void to_json(nlohmann::json& j, const DeterministicSoilSnapshot& value) {
    j = nlohmann::json{
        {"requestedWh", value.requestedWh},
        {"allocatedWh", value.allocatedWh},
        {"consumedWh", value.consumedWh},
        {"operationalStateCode", value.operationalStateCode},
        {"reliabilityIndex", value.reliabilityIndex},
        {"requestedBreakdownWh", value.requestedBreakdownWh},
        {"consumedBreakdownWh", value.consumedBreakdownWh}
    };
}

inline void to_json(nlohmann::json& j, const DeterministicStatePayload& value) {
    j = nlohmann::json{
        {"schemaVersion", value.schemaVersion},
        {"poolStorageWh", value.poolStorageWh},
        {"identity", value.identity},
        {"soil", value.soil}
    };
}

} // namespace Application::DTO
