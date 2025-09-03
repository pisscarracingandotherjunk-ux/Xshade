#pragma once

#include "../common/types.h"

namespace XShade {
    class XShadeCore {
    public:
        static bool Initialize();
        static void Shutdown();
        
        static void ToggleRTX();
        static void SetRTXEnabled(bool enabled);
        static bool IsRTXEnabled();
        
        static void UpdateRTXSettings(const RTXSettings& settings);
        static RTXSettings GetRTXSettings();

    private:
        static std::unique_ptr<ShaderManager> shader_manager_;
        static std::unique_ptr<DXHook> dx_hook_;
        static std::atomic<bool> initialized_;
        static std::atomic<RenderMode> current_mode_;
        static RTXSettings rtx_settings_;
        static std::mutex core_mutex_;
    };
    
    // C-style exports for external communication
    extern "C" {
        __declspec(dllexport) bool XShade_Initialize();
        __declspec(dllexport) void XShade_Shutdown();
        __declspec(dllexport) void XShade_ToggleRTX();
        __declspec(dllexport) void XShade_SetRTXEnabled(bool enabled);
        __declspec(dllexport) bool XShade_IsRTXEnabled();
    }
}
