include(FetchContent)

FetchContent_Declare(
  json
  URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)
FetchContent_MakeAvailable(json)

FetchContent_Declare(
  httplib
  GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
  GIT_TAG        v0.18.3
)
FetchContent_MakeAvailable(httplib)

find_package(SDL2 REQUIRED)

add_library(infrastructure
    window/Window.cpp
    threading/ThreadPool.cpp
    threading/CommandQueue.hpp
    filesystem/StdFileSystem.hpp
    llm/OllamaMockAdapter.cpp
    llm/OllamaAdapter.cpp
)

target_include_directories(infrastructure PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${SDL2_INCLUDE_DIRS}
)

find_package(Threads REQUIRED)

target_link_libraries(infrastructure PUBLIC 
    application 
    ${SDL2_LIBRARIES}
    Threads::Threads
    httplib::httplib
    nlohmann_json::nlohmann_json
)

target_compile_features(infrastructure PUBLIC cxx_std_20)
