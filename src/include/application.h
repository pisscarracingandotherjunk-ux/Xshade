#pragma once

#include "common/types.h"
#include "ui/system_tray.h"
#include "injection/process_manager.h"

namespace XShade {
    class Application {
    public:
        Application();
        ~Application();

        bool Initialize();
        void Shutdown();
        
        int Run();
        void RequestExit() { should_exit_ = true; }
        
        // Event handlers
        void OnToggleRTX();
        void OnShowSettings();
        void OnRobloxStateChanged(bool running);

    private:
        void RegisterHotkeys();
        void UnregisterHotkeys();
        void ProcessMessages();
        
        static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
        static DWORD WINAPI HotkeyThreadProc(LPVOID lpParameter);
        void HotkeyLoop();

        std::unique_ptr<SystemTray> system_tray_;
        std::unique_ptr<ProcessManager> process_manager_;
        
        ApplicationState app_state_;
        std::atomic<bool> should_exit_;
        HWND message_window_;
        HANDLE hotkey_thread_;
        std::atomic<bool> hotkey_active_;
    };
}
