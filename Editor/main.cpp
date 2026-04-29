#include "main.h"

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

} // namespace Editor

// OS entry point
int main() {
    Editor::EditorApp app;
    app.Run();
    return 0;
}