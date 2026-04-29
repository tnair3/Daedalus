#pragma once

#include "EngineCore.h"

namespace Editor {
    class EditorApp {
    public:
        EditorApp();
        ~EditorApp();

        void Run();

    private:
        bool m_IsRunning;
        Daedalus::EngineCore m_engine;
    };
}