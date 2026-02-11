include(FetchContent)
FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui
  GIT_TAG        v1.90.9-docking
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(imgui)

add_library(ui
    UI.cpp
    views/Hybrid2DView.cpp
    menus/MainMenu.cpp
    panels/AnalysisPanel.cpp
    panels/PatchAnalysisPanel.cpp
    panels/SettingsPanel.cpp
    panels/WelcomePanel.cpp
    panels/SoilSimPanel.cpp
    panels/VegetationDeclarationPanel.cpp
    panels/TerrainGeneratorPanel.cpp 
    panels/HydrologyPanel.cpp
    panels/TimelinePanel.cpp # Fourth Dimension
    panels/TimelinePanelHelpers.cpp
    panels/NarrativePanel.cpp # Observational
    panels/DiscursiveSystemPanel.cpp
    panels/RecommendationTrajectoryPanel.cpp
    panels/GlobalSynthesisPanel.cpp
    panels/AnalysisWorkspacePanel.cpp
    components/NarrativeGraphWidget.cpp
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    # ${imgui_SOURCE_DIR}/imgui_demo.cpp # Disabled for Production
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl2.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdlrenderer2.cpp
)

find_package(Vulkan REQUIRED)
find_package(SDL2 REQUIRED)

target_include_directories(ui PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${Vulkan_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIRS}
)

target_link_libraries(ui PUBLIC 
    application 
    world3d
    ${SDL2_LIBRARIES}
    Vulkan::Vulkan
)

target_compile_features(ui PUBLIC cxx_std_20)
