#pragma once

#include "../common/types.h"
#include <shellapi.h>

namespace XShade {
    class Application; // Forward declaration
    
    class SystemTray {
    public:
        SystemTray(Application* app);
        ~SystemTray();

        bool Initialize();
        void Shutdown();
        
        void UpdateIcon(RenderMode mode);
        void UpdateTooltip(const std::string& tooltip);
        void ShowNotification(const std::string& title, const std::string& message, DWORD timeout = 3000);

        // Context menu IDs
        enum MenuID {
            ID_TOGGLE_RTX = 1001,
            ID_SETTINGS = 1002,
            ID_ABOUT = 1003,
            ID_EXIT = 1004
        };

        static LRESULT CALLBACK TrayWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    private:
        void CreateContextMenu();
        void ShowContextMenu();
        void HandleMenuCommand(UINT commandId);
        
        Application* app_;
        NOTIFYICONDATAW nid_;
        HWND tray_window_;
        HMENU context_menu_;
        HICON vanilla_icon_;
        HICON rtx_icon_;
        bool initialized_;

        static constexpr UINT WM_TRAYICON = WM_USER + 1;
        static constexpr UINT TRAY_ID = 1;
    };
}
