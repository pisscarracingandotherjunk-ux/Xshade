#pragma once

#include "../common/types.h"
#include <tlhelp32.h>
#include <psapi.h>

namespace XShade {
    class ProcessManager {
    public:
        ProcessManager();
        ~ProcessManager();

        bool Initialize();
        void Shutdown();

        bool FindRobloxProcess();
        bool InjectDLL();
        bool EjectDLL();
        
        bool IsRobloxRunning() const;
        DWORD GetRobloxProcessId() const { return roblox_process_id_; }
        HWND GetRobloxWindow() const { return roblox_window_; }

        void StartProcessMonitoring();
        void StopProcessMonitoring();

        // Callback for when Roblox process state changes
        void SetProcessStateCallback(std::function<void(bool)> callback) {
            process_state_callback_ = callback;
        }

    private:
        bool IsProcessRunning(DWORD processId);
        bool InjectDLLIntoProcess(DWORD processId, const std::string& dllPath);
        std::vector<DWORD> FindProcessesByName(const std::wstring& processName);
        HWND FindRobloxWindow();
        
        static DWORD WINAPI MonitoringThreadProc(LPVOID lpParameter);
        void MonitoringLoop();

        DWORD roblox_process_id_;
        HWND roblox_window_;
        HANDLE roblox_process_handle_;
        HANDLE monitoring_thread_;
        std::atomic<bool> monitoring_active_;
        std::atomic<bool> dll_injected_;
        
        std::function<void(bool)> process_state_callback_;
        std::mutex process_mutex_;

        std::string dll_path_;
    };
}
