find_package(SDL2 REQUIRED)

add_library(infrastructure
    window/Window.cpp
    threading/ThreadPool.cpp
    filesystem/StdFileSystem.hpp
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
)

target_compile_features(infrastructure PUBLIC cxx_std_20)
