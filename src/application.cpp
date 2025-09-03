#include "include/application.h"
#include "include/ui/settings_dialog.h"
#include "include/logging/logger.h"
#include "include/config/configuration.h"

namespace XShade {
    Application::Application()
        : should_exit_(false)
        , message_window_(nullptr)
        , hotkey_thread_(nullptr)
        , hotkey_active_(false) {
    }

    Application::~Application() {
        Shutdown();
    }

    bool Application::Initialize() {
        // Initialize logging
        Logger::Initialize();
        LOG_INFO("XShade application starting...");

        // Load configuration
        Configuration& config = Configuration::Instance();
        if (!config.Load()) {
            LOG_WARNING("Failed to load configuration, using defaults");
        }

        // Initialize system tray
        system_tray_ = std::make_unique<SystemTray>(this);
        if (!system_tray_->Initialize()) {
            LOG_ERROR("Failed to initialize system tray");
            return false;
        }

        // Initialize process manager
        process_manager_ = std::make_unique<ProcessManager>();
        if (!process_manager_->Initialize()) {
            LOG_ERROR("Failed to initialize process manager");
            return false;
        }

        // Set up process monitoring callback
        process_manager_->SetProcessStateCallback([this](bool running) {
            OnRobloxStateChanged(running);
        });

        // Start monitoring for Roblox
        process_manager_->StartProcessMonitoring();

        // Register hotkeys
        RegisterHotkeys();

        // Update system tray
        RenderMode mode = config.GetRTXSettings().enabled ? RenderMode::RTX : RenderMode::Vanilla;
        app_state_.render_mode = mode;
        system_tray_->UpdateIcon(mode);

        LOG_INFO("XShade application initialized successfully");
        return true;
    }

    void Application::Shutdown() {
        LOG_INFO("XShade application shutting down...");

        // Stop hotkey thread
        UnregisterHotkeys();

        // Stop process monitoring
        if (process_manager_) {
            process_manager_->Shutdown();
            process_manager_.reset();
        }

        // Shutdown system tray
        if (system_tray_) {
            system_tray_->Shutdown();
            system_tray_.reset();
        }

        // Save configuration
        Configuration::Instance().Save();

        LOG_INFO("XShade application shut down successfully");
        Logger::Shutdown();
    }

    int Application::Run() {
        MSG msg = {};
        
        while (!should_exit_) {
            // Process Windows messages
            while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
                if (msg.message == WM_QUIT) {
                    should_exit_ = true;
                    break;
                }
                
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            
            Sleep(10); // Prevent 100% CPU usage
        }

        return 0;
    }

    void Application::OnToggleRTX() {
        RenderMode currentMode = app_state_.render_mode.load();
        RenderMode newMode = (currentMode == RenderMode::Vanilla) ? RenderMode::RTX : RenderMode::Vanilla;
        
        app_state_.render_mode = newMode;
        
        // Update configuration
        Configuration& config = Configuration::Instance();
        RTXSettings settings = config.GetRTXSettings();
        settings.enabled = (newMode == RenderMode::RTX);
        config.SetRTXSettings(settings);
        
        // Update system tray
        system_tray_->UpdateIcon(newMode);
        
        // Show notification
        std::string message = (newMode == RenderMode::RTX) ? "RTX Mode Enabled" : "Vanilla Mode Enabled";
        system_tray_->ShowNotification("XShade", message);
        
        // If Roblox is running, communicate with injected DLL
        if (app_state_.roblox_detected) {
            // This would normally communicate with the injected DLL
            // For now, we'll just log the change
            LOG_INFO("Render mode changed to: " + std::string(newMode == RenderMode::RTX ? "RTX" : "Vanilla"));
        }
        
        LOG_INFO("RTX toggled: " + std::string(newMode == RenderMode::RTX ? "enabled" : "disabled"));
    }

    void Application::OnShowSettings() {
        LOG_INFO("Opening settings dialog...");
        
        SettingsDialog dialog;
        dialog.SetRTXSettings(Configuration::Instance().GetRTXSettings());
        
        dialog.SetSettingsChangedCallback([this](const RTXSettings& settings) {
            // Update application state
            app_state_.render_mode = settings.enabled ? RenderMode::RTX : RenderMode::Vanilla;
            system_tray_->UpdateIcon(app_state_.render_mode);
            
            // Communicate changes to injected DLL if Roblox is running
            if (app_state_.roblox_detected) {
                LOG_INFO("Updated RTX settings in Roblox");
            }
        });
        
        if (dialog.Show()) {
            LOG_INFO("Settings dialog completed successfully");
        }
    }

    void Application::OnRobloxStateChanged(bool running) {
        app_state_.roblox_detected = running;
        
        if (running) {
            LOG_INFO("Roblox detected - attempting DLL injection");
            
            // Attempt DLL injection
            if (process_manager_->InjectDLL()) {
                system_tray_->ShowNotification("XShade", "Successfully attached to Roblox!");
                
                // Update tooltip
                RenderMode mode = app_state_.render_mode;
                std::string tooltip = std::string("XShade - ") + 
                                    (mode == RenderMode::RTX ? "RTX" : "Vanilla") + 
                                    " Mode (Active)";
                system_tray_->UpdateTooltip(tooltip);
                
                app_state_.roblox_window = process_manager_->GetRobloxWindow();
                app_state_.roblox_process_id = process_manager_->GetRobloxProcessId();
            } else {
                system_tray_->ShowNotification("XShade", "Failed to attach to Roblox", 5000);
                LOG_ERROR("Failed to inject DLL into Roblox process");
            }
        } else {
            LOG_INFO("Roblox process terminated");
            system_tray_->ShowNotification("XShade", "Roblox closed - XShade disconnected");
            
            // Reset state
            app_state_.roblox_window = nullptr;
            app_state_.roblox_process_id = 0;
            
            // Update tooltip
            RenderMode mode = app_state_.render_mode;
            std::string tooltip = std::string("XShade - ") + 
                                (mode == RenderMode::RTX ? "RTX" : "Vanilla") + 
                                " Mode (Waiting for Roblox)";
            system_tray_->UpdateTooltip(tooltip);
        }
    }

    void Application::RegisterHotkeys() {
        hotkey_active_ = true;
        hotkey_thread_ = CreateThread(nullptr, 0, HotkeyThreadProc, this, 0, nullptr);
        
        if (!hotkey_thread_) {
            LOG_ERROR("Failed to create hotkey thread");
            hotkey_active_ = false;
        } else {
            LOG_INFO("Hotkey monitoring started (F10 to toggle RTX)");
        }
    }

    void Application::UnregisterHotkeys() {
        if (!hotkey_active_) {
            return;
        }
        
        hotkey_active_ = false;
        
        if (hotkey_thread_) {
            WaitForSingleObject(hotkey_thread_, 2000);
            CloseHandle(hotkey_thread_);
            hotkey_thread_ = nullptr;
        }
        
        LOG_INFO("Hotkey monitoring stopped");
    }

    DWORD WINAPI Application::HotkeyThreadProc(LPVOID lpParameter) {
        Application* app = static_cast<Application*>(lpParameter);
        app->HotkeyLoop();
        return 0;
    }

    void Application::HotkeyLoop() {
        int toggleKey = Configuration::Instance().GetToggleHotkey();
        bool keyWasPressed = false;
        
        while (hotkey_active_) {
            bool keyIsPressed = (GetAsyncKeyState(toggleKey) & 0x8000) != 0;
            
            // Detect key press (transition from not pressed to pressed)
            if (keyIsPressed && !keyWasPressed) {
                OnToggleRTX();
            }
            
            keyWasPressed = keyIsPressed;
            Sleep(50); // Check hotkey state every 50ms
        }
    }
}
