find_package(glfw3 REQUIRED)
find_package(Vulkan REQUIRED)

set(EDITOR_SOURCES
        main.cpp
        main.h
        window.cpp
        window.h

        # imgui source files
        imgui/imgui.cpp
        imgui/imgui_demo.cpp
        imgui/imgui_draw.cpp
        imgui/imgui_tables.cpp
        imgui/imgui_widgets.cpp

        # imgui backends
        imgui/backends/imgui_impl_glfw.cpp
        imgui/backends/imgui_impl_vulkan.cpp
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
            ${Vulkan_INCLUDE_DIRS}
            imgui
            imgui/backends
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
            Vulkan::Vulkan
)

# Output to ../Build/
set_target_properties(EditorApp PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Build/bin"
)