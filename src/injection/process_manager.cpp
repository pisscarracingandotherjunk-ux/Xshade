#include "../include/injection/process_manager.h"
#include "../include/logging/logger.h"
#include <filesystem>

namespace XShade {
    ProcessManager::ProcessManager()
        : roblox_process_id_(0)
        , roblox_window_(nullptr)
        , roblox_process_handle_(nullptr)
        , monitoring_thread_(nullptr)
        , monitoring_active_(false)
        , dll_injected_(false) {
    }

    ProcessManager::~ProcessManager() {
        Shutdown();
    }

    bool ProcessManager::Initialize() {
        // Get DLL path
        char modulePath[MAX_PATH];
        GetModuleFileNameA(nullptr, modulePath, MAX_PATH);
        std::filesystem::path exePath(modulePath);
        dll_path_ = (exePath.parent_path() / DLL_NAME).string();

        if (!std::filesystem::exists(dll_path_)) {
            LOG_ERROR("XShadeCore.dll not found at: " + dll_path_);
            return false;
        }

        LOG_INFO("ProcessManager initialized. DLL path: " + dll_path_);
        return true;
    }

    void ProcessManager::Shutdown() {
        StopProcessMonitoring();
        
        if (dll_injected_) {
            EjectDLL();
        }

        if (roblox_process_handle_) {
            CloseHandle(roblox_process_handle_);
            roblox_process_handle_ = nullptr;
        }

        roblox_process_id_ = 0;
        roblox_window_ = nullptr;
    }

    bool ProcessManager::FindRobloxProcess() {
        std::lock_guard<std::mutex> lock(process_mutex_);

        auto processes = FindProcessesByName(ROBLOX_PROCESS_NAME);
        if (processes.empty()) {
            if (roblox_process_id_ != 0) {
                LOG_INFO("Roblox process no longer running");
                roblox_process_id_ = 0;
                roblox_window_ = nullptr;
                if (roblox_process_handle_) {
                    CloseHandle(roblox_process_handle_);
                    roblox_process_handle_ = nullptr;
                }
                dll_injected_ = false;
            }
            return false;
        }

        DWORD newProcessId = processes[0]; // Take the first one
        if (newProcessId != roblox_process_id_) {
            LOG_INFO("Found Roblox process: PID " + std::to_string(newProcessId));
            
            roblox_process_id_ = newProcessId;
            roblox_window_ = FindRobloxWindow();
            
            if (roblox_process_handle_) {
                CloseHandle(roblox_process_handle_);
            }
            
            roblox_process_handle_ = OpenProcess(
                PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | 
                PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ,
                FALSE, roblox_process_id_
            );

            if (!roblox_process_handle_) {
                LOG_ERROR("Failed to open Roblox process: " + std::to_string(GetLastError()));
                return false;
            }

            dll_injected_ = false;
        }

        return true;
    }

    bool ProcessManager::InjectDLL() {
        if (!FindRobloxProcess()) {
            LOG_ERROR("Cannot inject DLL: Roblox process not found");
            return false;
        }

        if (dll_injected_) {
            LOG_DEBUG("DLL already injected");
            return true;
        }

        std::lock_guard<std::mutex> lock(process_mutex_);
        
        if (InjectDLLIntoProcess(roblox_process_id_, dll_path_)) {
            dll_injected_ = true;
            LOG_INFO("Successfully injected XShadeCore.dll into Roblox");
            return true;
        }

        LOG_ERROR("Failed to inject DLL into Roblox process");
        return false;
    }

    bool ProcessManager::EjectDLL() {
        if (!dll_injected_) {
            return true;
        }

        // For now, we don't implement DLL ejection as it's complex and risky
        // The DLL will be unloaded when the process terminates
        LOG_INFO("DLL ejection not implemented - will be unloaded on process exit");
        dll_injected_ = false;
        return true;
    }

    bool ProcessManager::IsRobloxRunning() const {
        return roblox_process_id_ != 0 && IsProcessRunning(roblox_process_id_);
    }

    void ProcessManager::StartProcessMonitoring() {
        if (monitoring_active_) {
            return;
        }

        monitoring_active_ = true;
        monitoring_thread_ = CreateThread(nullptr, 0, MonitoringThreadProc, this, 0, nullptr);
        
        if (!monitoring_thread_) {
            LOG_ERROR("Failed to create monitoring thread");
            monitoring_active_ = false;
        } else {
            LOG_INFO("Process monitoring started");
        }
    }

    void ProcessManager::StopProcessMonitoring() {
        if (!monitoring_active_) {
            return;
        }

        monitoring_active_ = false;
        
        if (monitoring_thread_) {
            WaitForSingleObject(monitoring_thread_, 5000);
            CloseHandle(monitoring_thread_);
            monitoring_thread_ = nullptr;
        }

        LOG_INFO("Process monitoring stopped");
    }

    bool ProcessManager::IsProcessRunning(DWORD processId) {
        HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processId);
        if (!process) {
            return false;
        }

        DWORD exitCode;
        bool running = GetExitCodeProcess(process, &exitCode) && exitCode == STILL_ACTIVE;
        CloseHandle(process);
        
        return running;
    }

    bool ProcessManager::InjectDLLIntoProcess(DWORD processId, const std::string& dllPath) {
        HANDLE process = OpenProcess(
            PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | 
            PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ,
            FALSE, processId
        );

        if (!process) {
            LOG_ERROR("Failed to open target process for injection");
            return false;
        }

        // Allocate memory in target process for DLL path
        SIZE_T pathSize = dllPath.length() + 1;
        LPVOID remotePath = VirtualAllocEx(process, nullptr, pathSize, 
                                          MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        
        if (!remotePath) {
            LOG_ERROR("Failed to allocate memory in target process");
            CloseHandle(process);
            return false;
        }

        // Write DLL path to target process
        SIZE_T written;
        if (!WriteProcessMemory(process, remotePath, dllPath.c_str(), pathSize, &written)) {
            LOG_ERROR("Failed to write DLL path to target process");
            VirtualFreeEx(process, remotePath, 0, MEM_RELEASE);
            CloseHandle(process);
            return false;
        }

        // Get LoadLibraryA address
        HMODULE kernel32 = GetModuleHandleA("kernel32.dll");
        LPTHREAD_START_ROUTINE loadLibrary = (LPTHREAD_START_ROUTINE)GetProcAddress(kernel32, "LoadLibraryA");
        
        if (!loadLibrary) {
            LOG_ERROR("Failed to get LoadLibraryA address");
            VirtualFreeEx(process, remotePath, 0, MEM_RELEASE);
            CloseHandle(process);
            return false;
        }

        // Create remote thread to load DLL
        HANDLE remoteThread = CreateRemoteThread(process, nullptr, 0, loadLibrary, remotePath, 0, nullptr);
        
        if (!remoteThread) {
            LOG_ERROR("Failed to create remote thread for DLL injection");
            VirtualFreeEx(process, remotePath, 0, MEM_RELEASE);
            CloseHandle(process);
            return false;
        }

        // Wait for injection to complete
        WaitForSingleObject(remoteThread, INFINITE);

        // Get thread exit code (HMODULE of loaded DLL)
        DWORD exitCode;
        GetExitCodeThread(remoteThread, &exitCode);
        bool success = (exitCode != 0);

        // Cleanup
        CloseHandle(remoteThread);
        VirtualFreeEx(process, remotePath, 0, MEM_RELEASE);
        CloseHandle(process);

        return success;
    }

    std::vector<DWORD> ProcessManager::FindProcessesByName(const std::wstring& processName) {
        std::vector<DWORD> processes;
        
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot == INVALID_HANDLE_VALUE) {
            return processes;
        }

        PROCESSENTRY32W entry = {};
        entry.dwSize = sizeof(entry);

        if (Process32FirstW(snapshot, &entry)) {
            do {
                if (_wcsicmp(entry.szExeFile, processName.c_str()) == 0) {
                    processes.push_back(entry.th32ProcessID);
                }
            } while (Process32NextW(snapshot, &entry));
        }

        CloseHandle(snapshot);
        return processes;
    }

    HWND ProcessManager::FindRobloxWindow() {
        return FindWindowW(ROBLOX_WINDOW_CLASS, nullptr);
    }

    DWORD WINAPI ProcessManager::MonitoringThreadProc(LPVOID lpParameter) {
        ProcessManager* manager = static_cast<ProcessManager*>(lpParameter);
        manager->MonitoringLoop();
        return 0;
    }

    void ProcessManager::MonitoringLoop() {
        bool wasRunning = false;
        
        while (monitoring_active_) {
            bool isRunning = FindRobloxProcess();
            
            if (isRunning != wasRunning) {
                LOG_INFO(std::string("Roblox process state changed: ") + (isRunning ? "started" : "stopped"));
                
                if (process_state_callback_) {
                    process_state_callback_(isRunning);
                }
                
                wasRunning = isRunning;
            }

            Sleep(1000); // Check every second
        }
    }
}
