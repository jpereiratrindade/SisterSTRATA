add_executable(infrastructure_tests
    TestThreadPool.cpp
)

target_link_libraries(infrastructure_tests PRIVATE infrastructure)

add_test(NAME ThreadPoolTest COMMAND infrastructure_tests)
