add_library(core
    domain/Workspace.cpp
)

target_include_directories(core PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

target_compile_features(core PUBLIC cxx_std_20)
