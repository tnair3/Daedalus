set(ENGINE_SOURCES
        EngineCore.cpp
        ${CMAKE_SOURCE_DIR}/Include/EngineCore.h

        Graphics/Pipelines/TrianglePipeline.h
        Graphics/Pipelines/TrianglePipeline.cpp
)

# Static Library
add_library(Engine STATIC ${ENGINE_SOURCES})

# C++ Standard
set_target_properties(Engine PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
)

# Directories
target_include_directories(Engine
        PUBLIC ${CMAKE_SOURCE_DIR}/Include
        PRIVATE .
)

# Libraries
target_link_libraries(Engine
        PRIVATE
            glfw
            Vulkan::Vulkan)

# Output to ../Build/
set_target_properties(Engine PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Build/engine"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Build/engine"
)

# Spir-V Compilation
set(SHADER_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/Graphics/Shaders/triangle.vert
        ${CMAKE_CURRENT_SOURCE_DIR}/Graphics/Shaders/triangle.frag
)

set(SHADER_OUTPUT_DIR "${CMAKE_SOURCE_DIR}/Build/engine/shaders")
set(SPIRV_BINARY_FILES "")

foreach(SHADER ${SHADER_SOURCES})
    get_filename_component(FILE_NAME ${SHADER} NAME)
    set(SPIRV_OUTPUT "${SHADER_OUTPUT_DIR}/${FILE_NAME}.spv")

    add_custom_command(
            OUTPUT ${SPIRV_OUTPUT}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADER_OUTPUT_DIR}
            COMMAND Vulkan::glslc ${SHADER} -o ${SPIRV_OUTPUT}
            DEPENDS ${SHADER}
            COMMENT "Compiling Vulkan Shader: ${FILE_NAME} -> ${FILE_NAME}.spv"
    )

    list(APPEND SPIRV_BINARY_FILES ${SPIRV_OUTPUT})
endforeach()

add_custom_target(EngineShaders ALL DEPENDS ${SPIRV_BINARY_FILES})
add_dependencies(Engine EngineShaders)