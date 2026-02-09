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
    EXPECT_TRUE(discDTO.temporalContext.label.find("short") != std::string::npos);
    ASSERT_FALSE(discDTO.declaredProblems.empty());
    EXPECT_EQ(discDTO.declaredProblems[0], "Problem A");

    auto recDTO = Application::Mappers::IW::IWMapper::toRecommendationSnapshotDTO(j);
    ASSERT_TRUE(recDTO.has_value());
    // Should extract high confidence value from vector
    EXPECT_EQ(recDTO->recommendationText, "Analogy Primary");
}
