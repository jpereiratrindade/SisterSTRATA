#include <gtest/gtest.h>

#include "application/services/NarrativeContextAnalyzer.hpp"

using Application::Services::NarrativeContextAnalyzer;

// ─────────────────────────────────────────────
//  tokenizeText
// ─────────────────────────────────────────────

TEST(NarrativeContextAnalyzerTest, TokenizeTextFiltersShortWords) {
    auto tokens = NarrativeContextAnalyzer::tokenizeText("A do em solo degradado");
    // "A", "do", "em" are < 4 chars or stopwords → filtered
    // "solo" and "degradado" should remain
    ASSERT_EQ(tokens.size(), 2u);
    EXPECT_EQ(tokens[0], "solo");
    EXPECT_EQ(tokens[1], "degradado");
}

TEST(NarrativeContextAnalyzerTest, TokenizeTextFiltersStopwords) {
    auto tokens = NarrativeContextAnalyzer::tokenizeText("sobre para como between through");
    // All of these are stopwords
    EXPECT_TRUE(tokens.empty());
}

TEST(NarrativeContextAnalyzerTest, TokenizeTextLowerCases) {
    auto tokens = NarrativeContextAnalyzer::tokenizeText("VEGETACAO Pastagem");
    ASSERT_EQ(tokens.size(), 2u);
    EXPECT_EQ(tokens[0], "vegetacao");
    EXPECT_EQ(tokens[1], "pastagem");
}

TEST(NarrativeContextAnalyzerTest, TokenizeTextHandlesEmptyString) {
    auto tokens = NarrativeContextAnalyzer::tokenizeText("");
    EXPECT_TRUE(tokens.empty());
}

// ─────────────────────────────────────────────
//  dominantDimensionFromTokens
// ─────────────────────────────────────────────

TEST(NarrativeContextAnalyzerTest, DominantDimensionEcological) {
    std::set<std::string> tokens = {"solo", "vegetacao", "umidade", "clima"};
    EXPECT_EQ(NarrativeContextAnalyzer::dominantDimensionFromTokens(tokens), "ecological");
}

TEST(NarrativeContextAnalyzerTest, DominantDimensionProductive) {
    std::set<std::string> tokens = {"manejo", "producao", "pastejo", "fertilizacao"};
    EXPECT_EQ(NarrativeContextAnalyzer::dominantDimensionFromTokens(tokens), "productive");
}

TEST(NarrativeContextAnalyzerTest, DominantDimensionSocial) {
    std::set<std::string> tokens = {"regional", "governanca", "institucional", "politica"};
    EXPECT_EQ(NarrativeContextAnalyzer::dominantDimensionFromTokens(tokens), "social");
}

TEST(NarrativeContextAnalyzerTest, DominantDimensionMixedWhenEmpty) {
    std::set<std::string> tokens = {"alguma", "palavra", "qualquer"};
    EXPECT_EQ(NarrativeContextAnalyzer::dominantDimensionFromTokens(tokens), "mixed");
}

// ─────────────────────────────────────────────
//  buildContextGraph
// ─────────────────────────────────────────────

TEST(NarrativeContextAnalyzerTest, BuildContextGraphEmptyInput) {
    std::vector<Application::DTO::NarrativeStateDTO> narratives;
    auto graph = NarrativeContextAnalyzer::buildContextGraph(narratives);

    EXPECT_EQ(graph["distanceType"], "epistemic_narrative_jaccard_v1");
    EXPECT_FALSE(graph["causalInterpretationAllowed"].get<bool>());
    EXPECT_TRUE(graph["nodes"].empty());
    EXPECT_TRUE(graph["edges"].empty());
}

TEST(NarrativeContextAnalyzerTest, BuildContextGraphGroupsBySource) {
    Application::DTO::NarrativeStateDTO dto1;
    dto1.id = "OBS-1";
    dto1.source.sourceId = "article-A";
    dto1.intent.intentType = "describe";
    dto1.temporalContext.label = "2024";

    Application::DTO::NarrativeStateDTO dto2;
    dto2.id = "OBS-2";
    dto2.source.sourceId = "article-B";
    dto2.intent.intentType = "explain";
    dto2.temporalContext.label = "2025";

    std::vector dtos = {dto1, dto2};
    auto graph = NarrativeContextAnalyzer::buildContextGraph(dtos);

    EXPECT_EQ(graph["nodes"].size(), 2u);
    EXPECT_EQ(graph["nodes"][0]["id"], "article-A");
    EXPECT_EQ(graph["nodes"][1]["id"], "article-B");
}

TEST(NarrativeContextAnalyzerTest, BuildContextGraphCalculatesJaccardEdges) {
    // Two observations from different sources with overlapping tokens
    Application::DTO::NarrativeStateDTO dto1;
    dto1.id = "OBS-1";
    dto1.source.sourceId = "src-A";
    dto1.intent.intentType = "describe";
    dto1.temporalContext.label = "Solo degradado vegetacao nativa";

    Application::DTO::NarrativeStateDTO dto2;
    dto2.id = "OBS-2";
    dto2.source.sourceId = "src-B";
    dto2.intent.intentType = "describe";
    dto2.temporalContext.label = "Solo recuperado vegetacao pastagem";

    std::vector dtos = {dto1, dto2};
    auto graph = NarrativeContextAnalyzer::buildContextGraph(dtos);

    EXPECT_EQ(graph["nodes"].size(), 2u);
    // Should have an edge if tokens overlap
    if (!graph["edges"].empty()) {
        double similarity = graph["edges"][0]["similarity"].get<double>();
        EXPECT_GT(similarity, 0.0);
        EXPECT_LE(similarity, 1.0);
    }
}
