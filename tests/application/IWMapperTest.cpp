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
