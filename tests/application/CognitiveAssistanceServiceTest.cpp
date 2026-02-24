#include <gtest/gtest.h>

#include "application/services/cognitive/CognitiveAssistanceService.hpp"
#include "application/ports/ILLMService.hpp"
#include "application/dtos/cognitive/ContextBundleDTO.hpp"

#include <optional>

namespace {

class FakeLLMService final : public Application::Ports::ILLMService {
public:
    void requestCompletion(const std::vector<Application::Ports::LLMMessage>& messages,
                           CompletionCallback callback) override {
        lastMessages = messages;
        callback({true, "ok", ""});
    }

    bool isAvailable() const override { return true; }
    std::string getModelName() const override { return "fake"; }

    std::vector<Application::Ports::LLMMessage> lastMessages;
};

} // namespace

TEST(CognitiveAssistanceServiceTest, SnapshotSummaryReflectsBundleScope) {
    FakeLLMService llm;
    Application::Services::Cognitive::CognitiveAssistanceService service(&llm);

    Application::DTO::Cognitive::ContextBundleDTO bundle;
    bundle.bundleId = "BUNDLE-TEST";
    bundle.intent = "trajectory_reading";
    bundle.narratives = {"n1", "n2"};
    bundle.discursive = {"d1"};
    bundle.recommendation =
        "=> RECOMMENDATION SNAPSHOT [r1]\n"
        "x\n"
        "=> RECOMMENDATION SNAPSHOT [r2]\n";

    std::optional<Application::DTO::Cognitive::InterpretationSnapshotDTO> output;
    service.interpret(
        bundle,
        Application::Services::Cognitive::InterpretationMode::TrajectoryReading,
        [&](const auto& snapshot) { output = snapshot; });

    ASSERT_TRUE(output.has_value());
    EXPECT_NE(output->createdAt, "");
    EXPECT_NE(output->inputContextSummary.find("Context Bundle: BUNDLE-TEST"), std::string::npos);
    EXPECT_NE(output->inputContextSummary.find("narratives=2"), std::string::npos);
    EXPECT_NE(output->inputContextSummary.find("discursive=1"), std::string::npos);
    EXPECT_NE(output->inputContextSummary.find("recommendationSnapshots=2"), std::string::npos);

    ASSERT_EQ(llm.lastMessages.size(), 1u);
    EXPECT_NE(llm.lastMessages[0].content.find("SCOPE: narratives=2 discursive=1 recommendationSnapshots=2"),
              std::string::npos);
}

TEST(CognitiveAssistanceServiceTest, SnapshotSummaryIncludesRecordDescriptorsForTraceability) {
    FakeLLMService llm;
    Application::Services::Cognitive::CognitiveAssistanceService service(&llm);

    Application::DTO::Cognitive::ContextBundleDTO bundle;
    bundle.bundleId = "BUNDLE-TRACE";
    bundle.intent = "theme_analysis";
    bundle.narratives = {
        "--- OBSERVATION [candidate_abc] ---\n"
        "Source: 20260210_073058_coelho2018.pdf (2026-02-10T10:31:59Z)\n"
    };
    bundle.discursive = {
        "### DISCURSIVE SYSTEM [ds_1]\n"
        "Source Refs: 20260210_073058_coelho2018.pdf\n"
    };
    bundle.recommendation =
        "=> RECOMMENDATION SNAPSHOT [rec_9]\n"
        "Source: 20260210_073058_coelho2018.pdf (2026-02-10T10:31:59Z)\n";

    std::optional<Application::DTO::Cognitive::InterpretationSnapshotDTO> output;
    service.interpret(
        bundle,
        Application::Services::Cognitive::InterpretationMode::ThemeAnalysis,
        [&](const auto& snapshot) { output = snapshot; });

    ASSERT_TRUE(output.has_value());
    EXPECT_NE(output->inputContextSummary.find("NARRATIVE:candidate_abc"), std::string::npos);
    EXPECT_NE(output->inputContextSummary.find("DISCURSIVE:ds_1"), std::string::npos);
    EXPECT_NE(output->inputContextSummary.find("RECOMMENDATION:rec_9"), std::string::npos);
    EXPECT_NE(output->sourceBundleId.find("NARRATIVE:candidate_abc"), std::string::npos);
}
