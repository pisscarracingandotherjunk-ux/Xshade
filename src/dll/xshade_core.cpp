#include "../include/dll/xshade_core.h"
#include "../include/dll/dx_hook.h"
#include "../include/dll/shader_manager.h"
#include "../include/logging/logger.h"
#include "../include/config/configuration.h"

namespace XShade {
    // Static member definitions
    std::unique_ptr<ShaderManager> XShadeCore::shader_manager_;
    std::unique_ptr<DXHook> XShadeCore::dx_hook_;
    std::atomic<bool> XShadeCore::initialized_(false);
    std::atomic<RenderMode> XShadeCore::current_mode_(RenderMode::Vanilla);
    RTXSettings XShadeCore::rtx_settings_;
    std::mutex XShadeCore::core_mutex_;

    bool XShadeCore::Initialize() {
        std::lock_guard<std::mutex> lock(core_mutex_);
        
        if (initialized_) {
            return true;
        }

        // Initialize logging
        Logger::Initialize("xshade_core.log");
        Logger::SetLogLevel(LogLevel::Debug);
        LOG_INFO("XShade Core initializing...");

        // Load configuration
        Configuration& config = Configuration::Instance();
        if (!config.Load()) {
            LOG_WARNING("Failed to load configuration, using defaults");
        }
        
        rtx_settings_ = config.GetRTXSettings();
        current_mode_ = rtx_settings_.enabled ? RenderMode::RTX : RenderMode::Vanilla;

        // Initialize shader manager
        shader_manager_ = std::make_unique<ShaderManager>();
        
        // Initialize DirectX hooks
        dx_hook_ = std::make_unique<DXHook>();
        if (!dx_hook_->Initialize()) {
            LOG_ERROR("Failed to initialize DirectX hooks");
            return false;
        }

        // Set up hook callbacks
        dx_hook_->SetShaderManager(shader_manager_.get());
        
        // Install hooks
        if (!dx_hook_->InstallHooks()) {
            LOG_ERROR("Failed to install DirectX hooks");
            return false;
        }

        initialized_ = true;
        LOG_INFO("XShade Core initialized successfully");
        return true;
    }

    void XShadeCore::Shutdown() {
        std::lock_guard<std::mutex> lock(core_mutex_);
        
        if (!initialized_) {
            return;
        }

        LOG_INFO("XShade Core shutting down...");

        // Shutdown DirectX hooks
        if (dx_hook_) {
            dx_hook_->Shutdown();
            dx_hook_.reset();
        }

        // Shutdown shader manager
        if (shader_manager_) {
            shader_manager_->Shutdown();
            shader_manager_.reset();
        }

        // Save configuration
        Configuration& config = Configuration::Instance();
        config.SetRTXSettings(rtx_settings_);
        config.Save();

        initialized_ = false;
        LOG_INFO("XShade Core shut down successfully");
        Logger::Shutdown();
    }

    void XShadeCore::ToggleRTX() {
        RenderMode newMode = (current_mode_ == RenderMode::Vanilla) ? RenderMode::RTX : RenderMode::Vanilla;
        SetRTXEnabled(newMode == RenderMode::RTX);
    }

    void XShadeCore::SetRTXEnabled(bool enabled) {
        std::lock_guard<std::mutex> lock(core_mutex_);
        
        if (!initialized_) {
            return;
        }

        RenderMode newMode = enabled ? RenderMode::RTX : RenderMode::Vanilla;
        if (newMode == current_mode_) {
            return;
        }

        current_mode_ = newMode;
        rtx_settings_.enabled = enabled;

        if (shader_manager_) {
            shader_manager_->SetRenderMode(newMode);
        }

        LOG_INFO(std::string("RTX ") + (enabled ? "enabled" : "disabled"));
    }

    bool XShadeCore::IsRTXEnabled() {
        return current_mode_ == RenderMode::RTX;
    }

    void XShadeCore::UpdateRTXSettings(const RTXSettings& settings) {
        std::lock_guard<std::mutex> lock(core_mutex_);
        rtx_settings_ = settings;
        current_mode_ = settings.enabled ? RenderMode::RTX : RenderMode::Vanilla;
        
        if (shader_manager_) {
            shader_manager_->SetRenderMode(current_mode_);
        }

        LOG_INFO("RTX settings updated");
    }

    RTXSettings XShadeCore::GetRTXSettings() {
        std::lock_guard<std::mutex> lock(core_mutex_);
        return rtx_settings_;
    }

    // C-style exports
    extern "C" {
        __declspec(dllexport) bool XShade_Initialize() {
            return XShadeCore::Initialize();
        }

        __declspec(dllexport) void XShade_Shutdown() {
            XShadeCore::Shutdown();
        }

        __declspec(dllexport) void XShade_ToggleRTX() {
            XShadeCore::ToggleRTX();
        }

        __declspec(dllexport) void XShade_SetRTXEnabled(bool enabled) {
            XShadeCore::SetRTXEnabled(enabled);
        }

        __declspec(dllexport) bool XShade_IsRTXEnabled() {
            return XShadeCore::IsRTXEnabled();
        }
    }
}

// DLL Entry Point
BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved) {
    switch (dwReason) {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls(hModule);
            // Initialize in a separate thread to avoid blocking
            CreateThread(nullptr, 0, [](LPVOID) -> DWORD {
                XShade::XShadeCore::Initialize();
                return 0;
            }, nullptr, 0, nullptr);
            break;
            
        case DLL_PROCESS_DETACH:
            XShade::XShadeCore::Shutdown();
            break;
    }
    return TRUE;
}
