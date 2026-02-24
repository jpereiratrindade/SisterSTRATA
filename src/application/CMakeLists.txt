# Core Application Logic (Session, Services, Ports)
# Headless-safe
find_package(OpenSSL REQUIRED)

add_library(application
    Session.cpp
    services/IWIngestionService.cpp
    services/ProjectPersistenceService.cpp
    services/SimulationService.cpp
    services/NarrativeContextAnalyzer.cpp
)

target_include_directories(application PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(application PUBLIC 
    core
    infrastructure
    observational
    OpenSSL::Crypto
)
target_compile_features(application PUBLIC cxx_std_20)

# GUI Host Application (The "App" class that wires UI and World3D)
add_library(application_host
    Application.cpp
)

target_link_libraries(application_host PUBLIC
    application
    world3d
    ui
)
target_compile_features(application_host PUBLIC cxx_std_20)
