add_executable(application_tests
    TestUseCases.cpp
)

find_package(Threads REQUIRED)

target_link_libraries(application_tests PRIVATE application core)

add_test(NAME ApplicationUseCasesTest COMMAND application_tests)

add_executable(application_mapper_tests
    CognitiveAssistanceServiceTest.cpp
    CrossContextIsolationTest.cpp
    IngestionSynthesisReportTest.cpp
    InfrastructureResilienceRunTest.cpp
    MembraneContractEnforcementTest.cpp
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

function(add_application_mapper_gtest_ctest test_name gtest_filter)
  add_test(
    NAME ${test_name}
    COMMAND application_mapper_tests "--gtest_filter=${gtest_filter}"
  )
endfunction()

add_application_mapper_gtest_ctest(ApplicationMapper_CognitiveAssistance "CognitiveAssistanceServiceTest.*")
add_application_mapper_gtest_ctest(ApplicationMapper_CrossContextIsolation "CrossContextIsolationTest.*")
add_application_mapper_gtest_ctest(ApplicationMapper_IngestionSynthesis "IngestionSynthesisReportTest.*")
add_application_mapper_gtest_ctest(ApplicationMapper_InfrastructureResilience "InfrastructureResilienceRunTest.*")
add_application_mapper_gtest_ctest(ApplicationMapper_MembraneContract "MembraneContractEnforcementTest.*")
add_application_mapper_gtest_ctest(ApplicationMapper_ImpactProfileMapper "ImpactProfileMapperTest.*")
add_application_mapper_gtest_ctest(ApplicationMapper_IWMapper "IWMapperTest.*")
add_application_mapper_gtest_ctest(ApplicationMapper_NarrativeContextAnalyzer "NarrativeContextAnalyzerTest.*")
add_application_mapper_gtest_ctest(ApplicationMapper_SimulationService "SimulationServiceTest.*")
add_application_mapper_gtest_ctest(ApplicationMapper_ProjectPersistence "ProjectPersistenceServiceTest.*")
add_application_mapper_gtest_ctest(ApplicationMapper_IWIngestion "IWIngestionServiceTest.*")
