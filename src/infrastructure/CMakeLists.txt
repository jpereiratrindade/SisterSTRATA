include(FetchContent)

FetchContent_Declare(
  json
  URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(json)

FetchContent_Declare(
  httplib
  GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
  GIT_TAG        v0.18.3
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(httplib)

set(INFRASTRUCTURE_SOURCES
    threading/ThreadPool.cpp
    threading/CommandQueue.hpp
    filesystem/StdFileSystem.hpp
    llm/OllamaMockAdapter.cpp
    llm/OllamaAdapter.cpp
)

if(STRATA_BUILD_WORLD3D)
    find_package(SDL2 REQUIRED)
    list(APPEND INFRASTRUCTURE_SOURCES
        window/Window.cpp
    )
endif()

add_library(infrastructure
    ${INFRASTRUCTURE_SOURCES}
)

target_include_directories(infrastructure PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

if(STRATA_BUILD_WORLD3D)
    target_include_directories(infrastructure PUBLIC
        ${SDL2_INCLUDE_DIRS}
    )
endif()

find_package(Threads REQUIRED)

target_link_libraries(infrastructure PUBLIC 
    application 
    Threads::Threads
    httplib::httplib
    nlohmann_json::nlohmann_json
)

if(STRATA_BUILD_WORLD3D)
    target_link_libraries(infrastructure PUBLIC
        ${SDL2_LIBRARIES}
    )
endif()

target_compile_features(infrastructure PUBLIC cxx_std_20)
