find_package(Vulkan REQUIRED)
find_package(SDL2 REQUIRED)

add_library(world3d
    World3D.cpp
    Engine.cpp
    rendering/VulkanContext.cpp
    rendering/Swapchain.cpp
    rendering/VulkanRenderer.cpp
    rendering/Pipeline.cpp
    rendering/Buffer.cpp
    ScientificAdapter.cpp
    camera/Camera.cpp
    camera/CameraInputController.cpp
)

# Shader Compilation
find_program(GLSLC_EXECUTABLE glslc REQUIRED)

set(SHADERS
    shaders/simple.vert
    shaders/simple.frag
)

foreach(SHADER ${SHADERS})
    get_filename_component(FILENAME ${SHADER} NAME)
    set(SPV_FILE "${CMAKE_BINARY_DIR}/bin/shaders/${FILENAME}.spv")
    
    add_custom_command(
        OUTPUT ${SPV_FILE}
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/bin/shaders"
        COMMAND ${GLSLC_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/${SHADER}" -o "${SPV_FILE}"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${SHADER}"
        COMMENT "Compiling shader ${FILENAME}"
    )
    
    list(APPEND SPV_SHADERS ${SPV_FILE})
endforeach()

add_custom_target(Shaders DEPENDS ${SPV_SHADERS})
add_dependencies(world3d Shaders)

target_include_directories(world3d PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${Vulkan_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIRS}
)

target_link_libraries(world3d PUBLIC 
    core 
    infrastructure
    Vulkan::Vulkan
    ${SDL2_LIBRARIES}
)

target_compile_features(world3d PUBLIC cxx_std_20)
