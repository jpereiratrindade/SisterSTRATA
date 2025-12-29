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
