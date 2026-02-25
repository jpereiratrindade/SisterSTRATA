add_executable(core_tests
    TestWorkspace.cpp
)

target_link_libraries(core_tests PRIVATE core gtest gtest_main)

add_test(NAME CoreWorkspaceTest COMMAND core_tests)

add_executable(patch_analysis_tests
    PatchAnalysisTest.cpp
)
target_link_libraries(patch_analysis_tests PRIVATE core gtest gtest_main)
add_test(NAME CorePatchAnalysisTest COMMAND patch_analysis_tests)

add_executable(soil_rasterizer_tests
    SoilRasterizerTest.cpp
)
target_link_libraries(soil_rasterizer_tests PRIVATE core gtest gtest_main)
add_test(NAME CoreSoilRasterizerTest COMMAND soil_rasterizer_tests)

add_executable(patch_trajectory_test PatchTrajectoryTest.cpp)
target_link_libraries(patch_trajectory_test PRIVATE core gtest gtest_main)
add_test(NAME CorePatchTrajectoryTest COMMAND patch_trajectory_test)

add_executable(trajectory_lod_tests
    TestTrajectoryLOD.cpp
)
target_link_libraries(trajectory_lod_tests PRIVATE core gtest gtest_main)
add_test(NAME CoreTrajectoryLODTest COMMAND trajectory_lod_tests)

add_executable(veg_vector_tests
    VegetationSystemVectorTest.cpp
)
target_link_libraries(veg_vector_tests PRIVATE core gtest gtest_main)
add_test(NAME CoreVegVectorTest COMMAND veg_vector_tests)

add_executable(slope_fix_tests
    SlopeFixTest.cpp
)
target_link_libraries(slope_fix_tests PRIVATE core gtest gtest_main)
add_test(NAME CoreSlopeFixTest COMMAND slope_fix_tests)

add_executable(core_infrastructure_tests
    TestInfrastructure.cpp
)
target_link_libraries(core_infrastructure_tests PRIVATE core gtest gtest_main)
add_test(NAME CoreInfrastructureTest COMMAND core_infrastructure_tests)

add_executable(energy_allocation_policy_tests
    EnergyAllocationPolicyTest.cpp
)
target_link_libraries(energy_allocation_policy_tests PRIVATE core gtest gtest_main)
add_test(NAME CoreEnergyAllocationPolicyTest COMMAND energy_allocation_policy_tests)

add_executable(hydro_domain_tests
    HydroDomainTest.cpp
)
target_link_libraries(hydro_domain_tests PRIVATE core gtest gtest_main)
add_test(NAME CoreHydroDomainTest COMMAND hydro_domain_tests)

add_executable(environment_controller_determinism_tests
    EnvironmentControllerDeterminismTest.cpp
)
target_link_libraries(environment_controller_determinism_tests PRIVATE core gtest gtest_main)
add_test(NAME CoreEnvironmentControllerDeterminismTest COMMAND environment_controller_determinism_tests)
