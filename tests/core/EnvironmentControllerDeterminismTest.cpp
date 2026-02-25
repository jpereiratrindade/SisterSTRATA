#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "core/domain/energy/EnergyAllocationPolicy.hpp"
#include "core/domain/energy/EnergyPool.hpp"
#include "core/domain/identity/IdentityNode.hpp"
#include "core/domain/infrastructure/InfrastructureOrchestrator.hpp"
#include "core/domain/seto/SoilMonitorNode.hpp"
#include "core/domain/simulation/EnvironmentController.hpp"

namespace fs = std::filesystem;

namespace {

struct CsvRow {
    int day{0};
    double solarWh{0.0};
    double soilMoisture{0.0};
};

fs::path uniqueCsvPath(const std::string& label) {
    const auto stamp = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    return fs::temp_directory_path() /
           ("strata_environment_controller_" + label + "_" + std::to_string(stamp) + ".csv");
}

strata::domain::infrastructure::InfrastructureOrchestrator makeOrchestrator() {
    using namespace strata::domain;

    energy::EnergyPool pool(10000.0, 5000.0);
    energy::EqualitarianPolicy policy;
    identity::IdentityNode identityNode(20.0, 2.0, 5.0);
    seto::SoilMonitorNode soilNode(10.0, 40.0);

    return infrastructure::InfrastructureOrchestrator(
        pool,
        policy,
        identityNode,
        soilNode
    );
}

std::string readWholeFile(const fs::path& path) {
    std::ifstream in(path);
    EXPECT_TRUE(in.is_open());
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

std::vector<CsvRow> readRows(const fs::path& path) {
    std::ifstream in(path);
    EXPECT_TRUE(in.is_open());

    std::string line;
    std::getline(in, line); // header

    std::vector<CsvRow> rows;
    while (std::getline(in, line)) {
        if (line.empty()) {
            continue;
        }

        std::stringstream parser(line);
        std::string token;
        std::vector<std::string> tokens;
        while (std::getline(parser, token, ',')) {
            tokens.push_back(token);
        }

        // Expected CSV schema has 15 columns.
        EXPECT_GE(tokens.size(), 15u);
        if (tokens.size() < 15u) {
            continue;
        }

        CsvRow row;
        row.day = std::stoi(tokens[0]);
        row.solarWh = std::stod(tokens[1]);
        row.soilMoisture = std::stod(tokens[3]);
        rows.push_back(row);
    }

    return rows;
}

} // namespace

TEST(CoreEnvironmentControllerDeterminismTest, SameScenarioProducesIdenticalCsvOutput) {
    using namespace strata::domain;

    const fs::path csvA = uniqueCsvPath("normal_a");
    const fs::path csvB = uniqueCsvPath("normal_b");

    auto orchestratorA = makeOrchestrator();
    auto orchestratorB = makeOrchestrator();

    simulation::EnvironmentController controllerA(75, simulation::EnvironmentScenarioPreset::Normal);
    simulation::EnvironmentController controllerB(75, simulation::EnvironmentScenarioPreset::Normal);

    controllerA.run(orchestratorA, csvA.string());
    controllerB.run(orchestratorB, csvB.string());

    const std::string payloadA = readWholeFile(csvA);
    const std::string payloadB = readWholeFile(csvB);

    EXPECT_EQ(payloadA, payloadB);

    const auto rowsA = readRows(csvA);
    ASSERT_EQ(rowsA.size(), 75u);
    EXPECT_EQ(rowsA.front().day, 1);
    EXPECT_EQ(rowsA.back().day, 75);

    fs::remove(csvA);
    fs::remove(csvB);
}

TEST(CoreEnvironmentControllerDeterminismTest, SevereDroughtAlwaysReducesSolarAndMoistureAgainstNormal) {
    using namespace strata::domain;

    const fs::path csvNormal = uniqueCsvPath("normal");
    const fs::path csvDrought = uniqueCsvPath("drought");

    auto normalOrchestrator = makeOrchestrator();
    auto droughtOrchestrator = makeOrchestrator();

    simulation::EnvironmentController normalController(60, simulation::EnvironmentScenarioPreset::Normal);
    simulation::EnvironmentController droughtController(60, simulation::EnvironmentScenarioPreset::SevereDrought);

    normalController.run(normalOrchestrator, csvNormal.string());
    droughtController.run(droughtOrchestrator, csvDrought.string());

    const auto normalRows = readRows(csvNormal);
    const auto droughtRows = readRows(csvDrought);
    ASSERT_EQ(normalRows.size(), 60u);
    ASSERT_EQ(droughtRows.size(), normalRows.size());

    int strictSolarReductions = 0;
    int strictMoistureReductions = 0;
    for (size_t i = 0; i < normalRows.size(); ++i) {
        EXPECT_EQ(normalRows[i].day, droughtRows[i].day);
        EXPECT_LE(droughtRows[i].solarWh, normalRows[i].solarWh + 1e-9);
        EXPECT_LE(droughtRows[i].soilMoisture, normalRows[i].soilMoisture + 1e-9);
        EXPECT_GE(droughtRows[i].soilMoisture, -1e-9);
        EXPECT_LE(droughtRows[i].soilMoisture, 1.0 + 1e-9);

        if (droughtRows[i].solarWh + 1e-9 < normalRows[i].solarWh) {
            ++strictSolarReductions;
        }
        if (droughtRows[i].soilMoisture + 1e-9 < normalRows[i].soilMoisture) {
            ++strictMoistureReductions;
        }
    }

    EXPECT_GT(strictSolarReductions, 0);
    EXPECT_GT(strictMoistureReductions, 0);

    fs::remove(csvNormal);
    fs::remove(csvDrought);
}
