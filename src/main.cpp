#include "include/application.h"
#include "include/logging/logger.h"
#include <iostream>

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, 
                  _In_ LPSTR lpCmdLine, _In_ int nCmdShow) {
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);

    try {
        // Initialize COM for system tray and other Windows features
        CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

        // Create and initialize application
        XShade::Application app;
        if (!app.Initialize()) {
            XShade::Logger::Error("Failed to initialize XShade application");
            CoUninitialize();
            return -1;
        }

        // Check for administrator privileges
        BOOL isElevated = FALSE;
        HANDLE token = nullptr;
        
        if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token)) {
            TOKEN_ELEVATION elevation;
            DWORD size = sizeof(elevation);
            if (GetTokenInformation(token, TokenElevation, &elevation, sizeof(elevation), &size)) {
                isElevated = elevation.TokenIsElevated;
            }
            CloseHandle(token);
        }

        if (!isElevated) {
            XShade::Logger::Warning("XShade is not running with administrator privileges. "
                                   "Some features may not work properly.");
        }

        // Run main application loop
        XShade::Logger::Info("XShade started successfully");
        int result = app.Run();
        
        // Cleanup
        app.Shutdown();
        CoUninitialize();
        
        return result;
    }
    catch (const std::exception& e) {
        XShade::Logger::Error("Unhandled exception: " + std::string(e.what()));
        CoUninitialize();
        return -1;
    }
    catch (...) {
        XShade::Logger::Error("Unknown unhandled exception occurred");
        CoUninitialize();
        return -1;
    }
}

// Console entry point for debugging
int main() {
    return WinMain(GetModuleHandle(nullptr), nullptr, GetCommandLineA(), SW_HIDE);
}
