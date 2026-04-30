find_package(glfw3 REQUIRED)

set(EDITOR_SOURCES
        main.cpp
        main.h
        window.cpp
        window.h
)

# Executable
add_executable(EditorApp ${EDITOR_SOURCES})
# To hide the console, change add_executable to:
# add_executable(EditorApp WIN32 ${EDITOR_SOURCES})

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

if(WIN32)
    target_link_options(EditorApp
            PRIVATE
                "-static-libgcc"
                "-static-libstdc++"
                "-static"
    )
endif()

# Libraries
target_link_libraries(EditorApp
        PRIVATE
            Engine
            glfw
)

# Output to ../Build/
set_target_properties(EditorApp PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Build/bin"
)