add_library(application
    Session.cpp
)

# Since we don't have source files in the add_library call yet (if they are all headers),
# we might need at least one dummy or ensure "application" is an INTERFACE or has a source.
# Let's create a dummy source or better, implement Session.cpp to avoid "no sources" error.
# For now, I'll add a dummy file again or assume we will implement Session.cpp shortly.
# ACTUALLY, I will reimplement Session as .hpp/.cpp split to be clean.

target_include_directories(application PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(application PUBLIC core)

target_compile_features(application PUBLIC cxx_std_20)
