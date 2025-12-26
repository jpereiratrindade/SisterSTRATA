add_executable(core_tests
    TestWorkspace.cpp
)

target_link_libraries(core_tests PRIVATE core)

add_test(NAME CoreWorkspaceTest COMMAND core_tests)
