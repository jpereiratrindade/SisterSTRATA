#include <chrono>

#include <gtest/gtest.h>

#include "core/domain/vegetation/HypothesisID.hpp"

using Core::Domain::Vegetation::HypothesisID;

TEST(CoreHypothesisIdDeterminismTest, DefaultsToEpochTimestampForDeterministicConstruction) {
    const HypothesisID hypothesis("HYP-001");
    EXPECT_EQ(hypothesis.getValue(), "HYP-001");
    EXPECT_EQ(hypothesis.getTimestamp(), std::chrono::system_clock::time_point{});
}

TEST(CoreHypothesisIdDeterminismTest, PreservesExplicitTimestampWhenProvided) {
    const auto customTimestamp = std::chrono::system_clock::time_point{std::chrono::seconds{42}};
    const HypothesisID hypothesis("HYP-002", customTimestamp);
    EXPECT_EQ(hypothesis.getTimestamp(), customTimestamp);
}
