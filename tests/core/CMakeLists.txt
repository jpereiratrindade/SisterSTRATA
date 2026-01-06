add_executable(core_tests
    TestWorkspace.cpp
)

target_link_libraries(core_tests PRIVATE core)

add_test(NAME CoreWorkspaceTest COMMAND core_tests)

add_executable(patch_analysis_tests
    PatchAnalysisTest.cpp
)
target_link_libraries(patch_analysis_tests PRIVATE core)
add_test(NAME CorePatchAnalysisTest COMMAND patch_analysis_tests)

add_executable(soil_rasterizer_tests
    SoilRasterizerTest.cpp
)
target_link_libraries(soil_rasterizer_tests PRIVATE core)
add_test(NAME CoreSoilRasterizerTest COMMAND soil_rasterizer_tests)

add_executable(patch_trajectory_test PatchTrajectoryTest.cpp)
target_link_libraries(patch_trajectory_test PRIVATE core)
add_test(NAME CorePatchTrajectoryTest COMMAND patch_trajectory_test)

add_executable(trajectory_lod_tests
    TestTrajectoryLOD.cpp
)
target_link_libraries(trajectory_lod_tests PRIVATE core)
add_test(NAME CoreTrajectoryLODTest COMMAND trajectory_lod_tests)

add_executable(veg_vector_tests
    VegetationSystemVectorTest.cpp
)
target_link_libraries(veg_vector_tests PRIVATE core world3d)
add_test(NAME CoreVegVectorTest COMMAND veg_vector_tests)
