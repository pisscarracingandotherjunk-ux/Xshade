#include "../include/ui/system_tray.h"
#include "../include/application.h"
#include "../include/logging/logger.h"
#include <windowsx.h>

namespace XShade {
    SystemTray::SystemTray(Application* app)
        : app_(app)
        , tray_window_(nullptr)
        , context_menu_(nullptr)
        , vanilla_icon_(nullptr)
        , rtx_icon_(nullptr)
        , initialized_(false) {
        ZeroMemory(&nid_, sizeof(nid_));
    }

    SystemTray::~SystemTray() {
        Shutdown();
    }

    bool SystemTray::Initialize() {
        if (initialized_) {
            return true;
        }

        // Create a hidden window for tray messages
        WNDCLASSEXW wcex = {};
        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.lpfnWndProc = TrayWindowProc;
        wcex.hInstance = GetModuleHandle(nullptr);
        wcex.lpszClassName = L"XShadeTrayWindow";

        if (!RegisterClassExW(&wcex)) {
            LOG_ERROR("Failed to register tray window class");
            return false;
        }

        tray_window_ = CreateWindowExW(
            0,
            L"XShadeTrayWindow",
            L"XShade Tray",
            0, 0, 0, 0, 0,
            HWND_MESSAGE,
            nullptr,
            GetModuleHandle(nullptr),
            this
        );

        if (!tray_window_) {
            LOG_ERROR("Failed to create tray window");
            return false;
        }

        // Load icons (for now, use system icons)
        vanilla_icon_ = LoadIcon(nullptr, IDI_APPLICATION);
        rtx_icon_ = LoadIcon(nullptr, IDI_SHIELD);

        // Set up notification icon data
        nid_.cbSize = sizeof(NOTIFYICONDATAW);
        nid_.hWnd = tray_window_;
        nid_.uID = TRAY_ID;
        nid_.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_SHOWTIP;
        nid_.uCallbackMessage = WM_TRAYICON;
        nid_.hIcon = vanilla_icon_;
        wcscpy_s(nid_.szTip, L"XShade - Vanilla Mode");

        // Add to system tray
        if (!Shell_NotifyIconW(NIM_ADD, &nid_)) {
            LOG_ERROR("Failed to add system tray icon");
            return false;
        }

        // Create context menu
        CreateContextMenu();

        initialized_ = true;
        LOG_INFO("System tray initialized successfully");
        return true;
    }

    void SystemTray::Shutdown() {
        if (!initialized_) {
            return;
        }

        // Remove from system tray
        Shell_NotifyIconW(NIM_DELETE, &nid_);

        // Cleanup
        if (context_menu_) {
            DestroyMenu(context_menu_);
            context_menu_ = nullptr;
        }

        if (tray_window_) {
            DestroyWindow(tray_window_);
            tray_window_ = nullptr;
        }

        UnregisterClassW(L"XShadeTrayWindow", GetModuleHandle(nullptr));

        initialized_ = false;
        LOG_INFO("System tray shut down");
    }

    void SystemTray::UpdateIcon(RenderMode mode) {
        if (!initialized_) {
            return;
        }

        nid_.hIcon = (mode == RenderMode::RTX) ? rtx_icon_ : vanilla_icon_;
        const wchar_t* tooltip = (mode == RenderMode::RTX) ? L"XShade - RTX Mode" : L"XShade - Vanilla Mode";
        wcscpy_s(nid_.szTip, tooltip);

        Shell_NotifyIconW(NIM_MODIFY, &nid_);
    }

    void SystemTray::UpdateTooltip(const std::string& tooltip) {
        if (!initialized_) {
            return;
        }

        std::wstring wtooltip(tooltip.begin(), tooltip.end());
        wcscpy_s(nid_.szTip, wtooltip.c_str());
        Shell_NotifyIconW(NIM_MODIFY, &nid_);
    }

    void SystemTray::ShowNotification(const std::string& title, const std::string& message, DWORD timeout) {
        if (!initialized_) {
            return;
        }

        nid_.uFlags |= NIF_INFO;
        nid_.dwInfoFlags = NIIF_INFO;
        nid_.uTimeout = timeout;

        std::wstring wtitle(title.begin(), title.end());
        std::wstring wmessage(message.begin(), message.end());

        wcscpy_s(nid_.szInfoTitle, wtitle.c_str());
        wcscpy_s(nid_.szInfo, wmessage.c_str());

        Shell_NotifyIconW(NIM_MODIFY, &nid_);

        // Reset flags
        nid_.uFlags &= ~NIF_INFO;
    }

    void SystemTray::CreateContextMenu() {
        context_menu_ = CreatePopupMenu();
        if (!context_menu_) {
            LOG_ERROR("Failed to create context menu");
            return;
        }

        AppendMenuW(context_menu_, MF_STRING, ID_TOGGLE_RTX, L"Toggle RTX");
        AppendMenuW(context_menu_, MF_SEPARATOR, 0, nullptr);
        AppendMenuW(context_menu_, MF_STRING, ID_SETTINGS, L"Settings...");
        AppendMenuW(context_menu_, MF_STRING, ID_ABOUT, L"About XShade...");
        AppendMenuW(context_menu_, MF_SEPARATOR, 0, nullptr);
        AppendMenuW(context_menu_, MF_STRING, ID_EXIT, L"Exit");
    }

    void SystemTray::ShowContextMenu() {
        if (!context_menu_) {
            return;
        }

        POINT cursor;
        GetCursorPos(&cursor);

        // This is required for the menu to work properly
        SetForegroundWindow(tray_window_);

        TrackPopupMenu(context_menu_, TPM_RIGHTBUTTON, cursor.x, cursor.y, 0, tray_window_, nullptr);

        // Required to make the menu disappear when clicking elsewhere
        PostMessage(tray_window_, WM_NULL, 0, 0);
    }

    void SystemTray::HandleMenuCommand(UINT commandId) {
        switch (commandId) {
            case ID_TOGGLE_RTX:
                if (app_) {
                    app_->OnToggleRTX();
                }
                break;

            case ID_SETTINGS:
                if (app_) {
                    app_->OnShowSettings();
                }
                break;

            case ID_ABOUT:
                MessageBoxW(nullptr, 
                           L"XShade v1.0.0\n"
                           L"RTX Enhancement for Roblox\n\n"
                           L"Provides global RTX features including:\n"
                           L"• Global Illumination\n"
                           L"• Real-time Reflections\n"
                           L"• Enhanced Shadows\n"
                           L"• Ambient Occlusion",
                           L"About XShade", 
                           MB_OK | MB_ICONINFORMATION);
                break;

            case ID_EXIT:
                if (app_) {
                    app_->RequestExit();
                }
                break;
        }
    }

    LRESULT CALLBACK SystemTray::TrayWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        SystemTray* tray = reinterpret_cast<SystemTray*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));

        switch (uMsg) {
            case WM_CREATE: {
                CREATESTRUCT* cs = reinterpret_cast<CREATESTRUCT*>(lParam);
                tray = reinterpret_cast<SystemTray*>(cs->lpCreateParams);
                SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(tray));
                return 0;
            }

            case WM_TRAYICON:
                if (tray && LOWORD(lParam) == WM_RBUTTONUP) {
                    tray->ShowContextMenu();
                } else if (tray && LOWORD(lParam) == WM_LBUTTONDBLCLK) {
                    tray->HandleMenuCommand(ID_TOGGLE_RTX);
                }
                return 0;

            case WM_COMMAND:
                if (tray) {
                    tray->HandleMenuCommand(LOWORD(wParam));
                }
                return 0;

            default:
                return DefWindowProcW(hwnd, uMsg, wParam, lParam);
        }
    }
}
