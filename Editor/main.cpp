#include "main.h"
#include "window.h"

namespace Editor {

    EditorApp::EditorApp() : m_IsRunning(true) {
        // Initialize UI and Engine Core
        m_engine.Initialize();
    }

    EditorApp::~EditorApp() {
        // Clean up resources
    }

    void EditorApp::Run() {
        while (m_IsRunning) {
            // 1. Poll Events (Keyboard/Mouse)
            // 2. Update Engine Logic
            // 3. Render the Scene and UI
        }
    }

}

// OS entry point
int main() {
    // Create the window instance
    Daedalus::EditorWindow editor(1280, 720, "Daedalus Engine Editor - v1.0.0");

    // The Main Loop
    while (!editor.ShouldClose()) {
        // Handle input events (clicks, keypresses)
        editor.PollEvents();

        // Rendering logic will go here eventually
        // For now, it's just a blank, responsive window
    }

    return 0;
}