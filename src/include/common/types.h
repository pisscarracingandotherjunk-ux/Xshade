#pragma once

#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <atomic>
#include <mutex>

namespace XShade {
    // Forward declarations
    class Application;
    class ShaderManager;
    class RTXRenderer;
    class Configuration;

    // Enums
    enum class ShaderType {
        Vertex,
        Pixel,
        Compute,
        Geometry,
        Hull,
        Domain
    };

    enum class RenderMode {
        Vanilla,
        RTX
    };

    enum class RTXFeature {
        GlobalIllumination = 1 << 0,
        Reflections = 1 << 1,
        Shadows = 1 << 2,
        AmbientOcclusion = 1 << 3,
        All = GlobalIllumination | Reflections | Shadows | AmbientOcclusion
    };

    // Structures
    struct ShaderInfo {
        std::string name;
        ShaderType type;
        std::vector<uint8_t> bytecode;
        std::string source;
        ID3D11DeviceChild* shader = nullptr;
        ID3D11DeviceChild* rtx_shader = nullptr;
    };

    struct RTXSettings {
        bool enabled = false;
        RTXFeature features = RTXFeature::All;
        float global_illumination_intensity = 1.0f;
        float reflection_intensity = 1.0f;
        float shadow_quality = 1.0f;
        int bounce_count = 3;
        bool denoise_enabled = true;
    };

    struct ApplicationState {
        std::atomic<bool> running{ true };
        std::atomic<RenderMode> render_mode{ RenderMode::Vanilla };
        std::atomic<bool> roblox_detected{ false };
        HWND roblox_window = nullptr;
        DWORD roblox_process_id = 0;
    };

    // Type aliases
    using ShaderPtr = std::shared_ptr<ShaderInfo>;
    using ShaderMap = std::unordered_map<std::string, ShaderPtr>;
    using ToggleCallback = std::function<void(RenderMode)>;

    // Constants
    constexpr const char* APPLICATION_NAME = "XShade";
    constexpr const char* VERSION = "1.0.0";
    constexpr const wchar_t* ROBLOX_WINDOW_CLASS = L"WINDOWSCLIENT";
    constexpr const wchar_t* ROBLOX_PROCESS_NAME = L"RobloxPlayerBeta.exe";
    constexpr const char* DLL_NAME = "XShadeCore.dll";
    constexpr const char* CONFIG_FILE = "xshade.ini";
}

// Utility macros
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p) = nullptr; } }
#define CHECK_HR(hr) { if(FAILED(hr)) { return hr; } }
#define LOG_ERROR(msg) XShade::Logger::Error(msg)
#define LOG_INFO(msg) XShade::Logger::Info(msg)
#define LOG_DEBUG(msg) XShade::Logger::Debug(msg)
