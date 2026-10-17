set(EDITOR_SOURCES
        Main.cpp
        Main.h
        Window.cpp
        Window.h
        #EditorVulkanBootstrap.cpp

        # imgui source files
        ${CMAKE_SOURCE_DIR}/ThirdParty/imgui/imgui.cpp
        ${CMAKE_SOURCE_DIR}/ThirdParty/imgui/imgui_demo.cpp
        ${CMAKE_SOURCE_DIR}/ThirdParty/imgui/imgui_draw.cpp
        ${CMAKE_SOURCE_DIR}/ThirdParty/imgui/imgui_tables.cpp
        ${CMAKE_SOURCE_DIR}/ThirdParty/imgui/imgui_widgets.cpp

        # imgui backends
        ${CMAKE_SOURCE_DIR}/ThirdParty/imgui/backends/imgui_impl_glfw.cpp
        ${CMAKE_SOURCE_DIR}/ThirdParty/imgui/backends/imgui_impl_vulkan.cpp
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
    get_filename_component(COMPILER_BIN_DIR ${CMAKE_CXX_COMPILER} DIRECTORY)

    add_custom_command(TARGET EditorApp POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${COMPILER_BIN_DIR}/libwinpthread-1.dll" "$<TARGET_FILE_DIR:EditorApp>"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${COMPILER_BIN_DIR}/libgcc_s_seh-1.dll" "$<TARGET_FILE_DIR:EditorApp>"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${COMPILER_BIN_DIR}/libstdc++-6.dll" "$<TARGET_FILE_DIR:EditorApp>"
            VERBATIM
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
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Build/editor"
)