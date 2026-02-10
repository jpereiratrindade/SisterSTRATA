#include <gtest/gtest.h>
#include "application/mappers/IWMapper.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class IWMapperTest : public ::testing::Test {
protected:
    void SetUp() override {}
};

TEST_F(IWMapperTest, ParseDiscursiveSystem) {
    // Mock JSON similar to composicao.pdf.json
    json j = R"({
      "discursiveSystem": {
        "declaredProblems": [
          { "statement": "Problem A" }
        ],
        "declaredActions": [
          { "statement": "Action B" }
        ]
      },
      "allegedMechanisms": [
        "Mechanism C"
      ]
    })"_json;

    auto dto = Application::Mappers::IW::IWMapper::toDiscursiveSystemDTO(j);

    ASSERT_EQ(dto.declaredProblems.size(), 1);
    EXPECT_EQ(dto.declaredProblems[0], "Problem A");
    ASSERT_EQ(dto.declaredActions.size(), 1);
    EXPECT_EQ(dto.declaredActions[0], "Action B");
    ASSERT_EQ(dto.allegedMechanisms.size(), 1);
    EXPECT_EQ(dto.allegedMechanisms[0], "Mechanism C");
}

TEST_F(IWMapperTest, ParseRecommendation) {
    json j = R"({
      "trajectoryAnalogies": [
        {
          "analogy": "Analogy Text",
          "justification": "Justification Text",
          "scope": "Future"
        }
      ]
    })"_json;

    auto dtoOpt = Application::Mappers::IW::IWMapper::toRecommendationSnapshotDTO(j);
    ASSERT_TRUE(dtoOpt.has_value());
    EXPECT_EQ(dtoOpt->recommendationText, "Analogy Text");
    EXPECT_EQ(dtoOpt->expectedOutcome, "Justification Text");
    ASSERT_FALSE(dtoOpt->contextConditions.empty());
    EXPECT_EQ(dtoOpt->contextConditions[0], "Future");
}

TEST_F(IWMapperTest, ParseMultipleRecommendations) {
    json j = R"({
      "source": {
        "artifactId": "bundle-a"
      },
      "trajectoryAnalogies": [
        {
          "analogy": "Analogy 1",
          "justification": "Justification 1",
          "scope": "Local"
        },
        {
          "analogy": "Analogy 2",
          "justification": "Justification 2",
          "scope": "Regional"
        }
      ]
    })"_json;

    auto dtos = Application::Mappers::IW::IWMapper::toRecommendationSnapshotDTOs(j);
    ASSERT_EQ(dtos.size(), 2);
    EXPECT_EQ(dtos[0].recommendationText, "Analogy 1");
    EXPECT_EQ(dtos[1].recommendationText, "Analogy 2");
    EXPECT_EQ(dtos[0].sourceReference.sourceId, "bundle-a");
}

TEST_F(IWMapperTest, ParseEmptyNarrative) {
    json j = R"({
      "narrativeObservations": []
    })"_json;

    auto dtos = Application::Mappers::IW::IWMapper::toNarrativeStateDTOs(j);
    EXPECT_TRUE(dtos.empty());
}

TEST_F(IWMapperTest, ParseComplexInputs) {
    // Test pipe-separated values and candidate vectors
    json j = R"({
      "sourceProfile": {
        "temporalScale": "short|medium|long" 
      },
      "discursiveSystem": {
        "declaredProblems": [
          { "statement": "Problem A|Problem B" }
        ]
      },
      "trajectoryAnalogies": [
         {
             "analogy": [ {"value": "Analogy Primary", "confidence": 0.9}, {"value": "Analogy Secondary", "confidence": 0.4} ],
             "scope": "Global"
         }
      ]
    })"_json;

    auto discDTO = Application::Mappers::IW::IWMapper::toDiscursiveSystemDTO(j);
    // Should extract first part of pipe
    ASSERT_FALSE(discDTO.declaredProblems.empty());
    EXPECT_EQ(discDTO.declaredProblems[0], "Problem A");

    auto recDTO = Application::Mappers::IW::IWMapper::toRecommendationSnapshotDTO(j);
    ASSERT_TRUE(recDTO.has_value());
    // Should extract high confidence value from vector
    EXPECT_EQ(recDTO->recommendationText, "Analogy Primary");
}

TEST_F(IWMapperTest, ParseNarrativeHistoryWithNamespacedMetadata) {
    json j = R"({
      "source": {
        "artifactId": "bundle-history"
      },
      "history": [
        {
          "id": "candidate-1",
          "source": {
            "type": 3,
            "sourceId": "src-a",
            "productionDate": "2026-02-10T10:44:44Z"
          },
          "temporalContext": {
            "category": 3,
            "label": "Context Label"
          },
          "intent": {
            "type": 0
          },
          "metadata": {
            "observation": "Observed text",
            "sourceSection": "Results",
            "pageRange": "pp. 3-4"
          }
        }
      ]
    })"_json;

    auto dtos = Application::Mappers::IW::IWMapper::toNarrativeStateDTOs(j);
    ASSERT_EQ(dtos.size(), 1);
    EXPECT_EQ(dtos[0].id, "candidate-1");
    EXPECT_EQ(dtos[0].source.sourceType, "SCIENTIFIC_ARTICLE");
    EXPECT_EQ(dtos[0].metadata.at("iw.observation"), "Observed text");
    EXPECT_EQ(dtos[0].metadata.at("iw.sourceSection"), "Results");
    EXPECT_EQ(dtos[0].metadata.at("iw.pageRange"), "pp. 3-4");
}

TEST_F(IWMapperTest, ParseDiscursiveSystemsArray) {
    json j = R"({
      "systems": [
        {
          "id": "ds-1",
          "declaredProblems": [{"statement": "Problem 1"}],
          "declaredActions": [{"statement": "Action 1"}],
          "allegedMechanisms": [{"statement": "Mechanism 1"}],
          "expectedEffects": [{"statement": "Effect 1"}],
          "sourceReferences": [
            {
              "type": 4,
              "sourceId": "artifact-a",
              "productionDate": "2026-02-10T10:44:44Z"
            }
          ],
          "temporalContext": {
            "category": 3,
            "label": "general"
          }
        }
      ]
    })"_json;

    auto dtos = Application::Mappers::IW::IWMapper::toDiscursiveSystemDTOs(j);
    ASSERT_EQ(dtos.size(), 1);
    EXPECT_EQ(dtos[0].id, "ds-1");
    EXPECT_EQ(dtos[0].declaredProblems[0], "Problem 1");
    EXPECT_EQ(dtos[0].sourceReferences[0].sourceType, "REPORT");
    EXPECT_EQ(dtos[0].temporalContext.category, "CONTEMPORARY");
}
