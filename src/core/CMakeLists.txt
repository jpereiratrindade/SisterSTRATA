add_library(core
    domain/Workspace.cpp
    domain/soils/SoilSystem.cpp
    domain/analysis/PatchAnalysis.cpp
    domain/analysis/SoilRasterizer.cpp
    domain/hydro/DrainageSystem.cpp
    domain/hydro/Watershed.cpp
    domain/hydro/HydrologyReport.cpp
)

target_include_directories(core PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

target_compile_features(core PUBLIC cxx_std_20)
