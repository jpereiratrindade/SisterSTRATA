# Core Application Logic (Session, Services, Ports)
# Headless-safe
add_library(application
    Session.cpp
)

target_include_directories(application PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(application PUBLIC 
    core
    infrastructure
    observational
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
