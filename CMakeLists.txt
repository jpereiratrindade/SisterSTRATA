cmake_minimum_required(VERSION 3.20)

project(SisterSTRATA VERSION 1.10.2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(STRATA_BUILD_WORLD3D "Build Vulkan World3D/UI modules and desktop app target" ON)

enable_testing()

# Output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Add subdirectories
add_subdirectory(src/core)
add_subdirectory(src/infrastructure)
add_subdirectory(src/application)
if(STRATA_BUILD_WORLD3D)
    add_subdirectory(src/world3d)
    add_subdirectory(src/ui)
endif()
add_subdirectory(src/observational)
add_subdirectory(tests)
add_subdirectory(src/cli)

if(STRATA_BUILD_WORLD3D)
    # Entry point
    add_executable(SisterSTRATA src/main.cpp)

    # Link modules to executable
    target_link_libraries(SisterSTRATA PRIVATE
        application_host
        core
        application
        infrastructure
        ui
        world3d
    )
endif()
