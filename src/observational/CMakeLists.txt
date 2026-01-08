add_library(observational INTERFACE)

target_include_directories(observational INTERFACE
    ${CMAKE_SOURCE_DIR}
)

target_compile_features(observational INTERFACE cxx_std_20)

# Optional: Attach sources for IDE visibility if using newer CMake, 
# but strictly speaking INTERFACE libs don't compile sources.
# keeping it simple for now.

target_link_libraries(observational INTERFACE
    nlohmann_json::nlohmann_json
)
