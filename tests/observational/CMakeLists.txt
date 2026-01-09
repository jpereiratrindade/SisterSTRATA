find_package(Threads REQUIRED)

# Legacy tests (Manual main)
add_executable(observational_tests_legacy
    NarrativeObservationTest.cpp
)
target_link_libraries(observational_tests_legacy
    PRIVATE
    observational
)
add_test(NAME ObservationalNarrativeTestLegacy COMMAND observational_tests_legacy)

# New GTest-based tests
add_executable(observational_impact_tests
    impact_profile/TrajectoryImpactProfileTest.cpp
    impact_profile/TrajectoryImpactAnalyzerTest.cpp
)
target_link_libraries(observational_impact_tests
    PRIVATE
    observational
    core
    gtest_main
    gtest
    Threads::Threads
)
add_test(NAME ObservationalImpactTest COMMAND observational_impact_tests)
