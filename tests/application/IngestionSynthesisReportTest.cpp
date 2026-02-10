#include <gtest/gtest.h>

#include "application/Session.hpp"
#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>
#include <string>
#include <chrono>

namespace fs = std::filesystem;
using json = nlohmann::json;

namespace {

void writeTextFile(const fs::path& path, const std::string& content) {
    fs::create_directories(path.parent_path());
    std::ofstream out(path);
    out << content;
}

fs::path uniqueTempRoot() {
    const auto stamp = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    return fs::temp_directory_path() / ("strata_ingest_report_test_" + std::to_string(stamp));
}

} // namespace

TEST(IngestionSynthesisReportTest, WritesCanonicalJsonAndMarkdownAfterDirectoryIngest) {
    const fs::path tempRoot = uniqueTempRoot();
    const fs::path iwRoot = tempRoot / "iw";
    const fs::path bundleDir = iwRoot / "strata" / "consumables" / "bundle-a";
    const fs::path projectRoot = tempRoot / "project";

    const std::string trajectoryJson = R"({
  "source": {
    "artifactId": "bundle-a",
    "ingestedAt": "2026-02-10T11:06:37Z"
  },
  "trajectoryAnalogies": [
    {
      "analogy": "Analogy A",
      "justification": "Justification A",
      "scope": "temporal"
    }
  ]
})";
    writeTextFile(bundleDir / "TrajectoryAnalogies.json", trajectoryJson);

    Application::Session session;
    session.setProjectRoot(projectRoot.string());
    session.ingestFromIWDirectory(iwRoot.string());

    const fs::path reportDir = projectRoot / "reports" / "ingestion";
    const fs::path latestJson = reportDir / "IngestionSynthesisReport.latest.json";
    const fs::path latestMd = reportDir / "IngestionSynthesisReport.latest.md";

    ASSERT_TRUE(fs::exists(latestJson));
    ASSERT_TRUE(fs::exists(latestMd));

    std::ifstream in(latestJson);
    ASSERT_TRUE(in.is_open());
    json report;
    in >> report;

    EXPECT_EQ(report.value("reportType", ""), "IngestionSynthesisReport");
    EXPECT_EQ(report["summary"].value("bundlesDetected", 0), 0);
    EXPECT_EQ(report["summary"].value("bundlesIngested", 0), 0);
    EXPECT_EQ(report["summary"].value("standaloneFiles", 0), 1);
    EXPECT_EQ(report["contexts"]["recommendation"].value("mapped", 0), 1);
    EXPECT_EQ(report["epistemicStatus"].value("allowsResilienceInference", true), false);
    ASSERT_TRUE(report.contains("narrativeContextGraph"));
    EXPECT_EQ(report["narrativeContextGraph"].value("distanceType", ""), "epistemic_narrative_jaccard_v1");
    EXPECT_GE(report["narrativeContextGraph"]["nodes"].size(), 0);
    EXPECT_GE(report["narrativeContextGraph"]["edges"].size(), 0);

    fs::remove_all(tempRoot);
}
