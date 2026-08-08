set(EDITOR_SOURCES
        Main.cpp
        Main.h
        Window.cpp
        Window.h
        EditorVulkanBootstrap.cpp

        # imgui source files
        ../ThirdParty/imgui/imgui.cpp
        ../ThirdParty/imgui/imgui_demo.cpp
        ../ThirdParty/imgui/imgui_draw.cpp
        ../ThirdParty/imgui/imgui_tables.cpp
        ../ThirdParty/imgui/imgui_widgets.cpp

        # imgui backends
        ../ThirdParty/imgui/backends/imgui_impl_glfw.cpp
        ../ThirdParty/imgui/backends/imgui_impl_vulkan.cpp
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
            ${CMAKE_SOURCE_DIR}/ThirdParty/imgui
            ${CMAKE_SOURCE_DIR}/ThirdParty/imgui/backends
            ${Vulkan_INCLUDE_DIRS}
)

if(WIN32)
    target_link_options(EditorApp
            PRIVATE
                "-static-libgcc"
                "-static-libstdc++"
    )
endif()

# Libraries
target_link_libraries(EditorApp
        PRIVATE
            Engine
            glfw
            Vulkan::Vulkan
            nfd::nfd
)

# Output to ../Build/
set_target_properties(EditorApp PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Build/bin"
)