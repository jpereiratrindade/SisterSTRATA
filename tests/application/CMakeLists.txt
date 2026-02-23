add_executable(application_tests
    TestUseCases.cpp
)

find_package(Threads REQUIRED)

target_link_libraries(application_tests PRIVATE application core)

add_test(NAME ApplicationUseCasesTest COMMAND application_tests)

add_executable(application_mapper_tests
    CognitiveAssistanceServiceTest.cpp
    IngestionSynthesisReportTest.cpp
    InfrastructureResilienceRunTest.cpp
    ImpactProfileMapperTest.cpp
    IWMapperTest.cpp
    NarrativeContextAnalyzerTest.cpp
    SimulationServiceTest.cpp
    ProjectPersistenceServiceTest.cpp
    IWIngestionServiceTest.cpp
)
target_link_libraries(application_mapper_tests 
    PRIVATE 
    application 
    core
    observational
    gtest 
    gtest_main
    Threads::Threads
)
target_include_directories(application_mapper_tests PRIVATE ${CMAKE_SOURCE_DIR})
add_test(NAME ApplicationMapperTest COMMAND application_mapper_tests)
