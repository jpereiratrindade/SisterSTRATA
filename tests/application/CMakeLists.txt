add_executable(application_tests
    TestUseCases.cpp
)

target_link_libraries(application_tests PRIVATE application core)

add_test(NAME ApplicationUseCasesTest COMMAND application_tests)
