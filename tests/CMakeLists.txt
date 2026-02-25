include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
# Force shared CRT to match standard practice (though strictly project dependent)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

add_subdirectory(core)
add_subdirectory(observational)
add_subdirectory(application)
add_subdirectory(infrastructure)

add_test(
  NAME MembraneDependencyGuard
  COMMAND bash ${CMAKE_SOURCE_DIR}/scripts/check_membrane_dependencies.sh ${CMAKE_SOURCE_DIR}
)

add_test(
  NAME CoreDomainBoundaryGuard
  COMMAND bash ${CMAKE_SOURCE_DIR}/scripts/check_core_domain_dependencies.sh ${CMAKE_SOURCE_DIR}
)

add_test(
  NAME ExceptionBoundaryGuard
  COMMAND bash ${CMAKE_SOURCE_DIR}/scripts/check_exception_boundaries.sh ${CMAKE_SOURCE_DIR}
)

add_test(
  NAME GitArtifactHygieneGuard
  COMMAND bash ${CMAKE_SOURCE_DIR}/scripts/check_git_artifact_hygiene.sh ${CMAKE_SOURCE_DIR}
)
