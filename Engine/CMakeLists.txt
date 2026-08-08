set(ENGINE_SOURCES
        EngineCore.cpp
        ${CMAKE_SOURCE_DIR}/Include/EngineCore.h
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
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Build/lib"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Build/lib"
)