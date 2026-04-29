set(EDITOR_SOURCES
        main.cpp
        main.h
)

# Executable
add_executable(EditorApp ${EDITOR_SOURCES})

# C++ Standard
set_target_properties(EditorApp PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
)

# Directories
target_include_directories(EditorApp
        PRIVATE
            .
            ${CMAKE_SOURCE_DIR}/Include
)

# Libraries
target_link_libraries(EditorApp PRIVATE Engine)

# Output to ../Build/
set_target_properties(EditorApp PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Build/bin"
)