add_executable(sister_strata_cli
    main_cli.cpp
)

target_link_libraries(sister_strata_cli PRIVATE
    application # The headless session lib
    core
    infrastructure
    observational
)

target_compile_features(sister_strata_cli PRIVATE cxx_std_20)
