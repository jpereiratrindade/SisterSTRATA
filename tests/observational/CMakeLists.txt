add_executable(observational_tests
    NarrativeObservationTest.cpp
)

target_link_libraries(observational_tests
    PRIVATE
    observational
    # Explicitly NOT linking core
)

add_test(NAME ObservationalNarrativeTest COMMAND observational_tests)
