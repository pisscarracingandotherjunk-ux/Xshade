#include "../include/dll/dx_hook.h"
#include "../include/dll/shader_manager.h"
#include "../include/logging/logger.h"
#include <detours.h>

#pragma comment(lib, "detours.lib")

namespace XShade {
    // Static member definitions
    D3D11CreateDeviceAndSwapChain_t DXHook::original_create_device_and_swap_chain_ = nullptr;
    VSSetShader_t DXHook::original_vs_set_shader_ = nullptr;
    PSSetShader_t DXHook::original_ps_set_shader_ = nullptr;
    Present_t DXHook::original_present_ = nullptr;

    void** DXHook::device_context_vtable_ = nullptr;
    void** DXHook::swap_chain_vtable_ = nullptr;
    bool DXHook::hooks_installed_ = false;
    ShaderManager* DXHook::shader_manager_ = nullptr;
    ToggleCallback DXHook::toggle_callback_;
    ID3D11Device* DXHook::hooked_device_ = nullptr;
    std::mutex DXHook::hook_mutex_;

    bool DXHook::Initialize() {
        LOG_INFO("Initializing DirectX hooks...");
        
        // Get D3D11 module
        HMODULE d3d11Module = GetModuleHandleA("d3d11.dll");
        if (!d3d11Module) {
            LOG_ERROR("Failed to get d3d11.dll module handle");
            return false;
        }

        // Get D3D11CreateDeviceAndSwapChain function
        original_create_device_and_swap_chain_ = (D3D11CreateDeviceAndSwapChain_t)
            GetProcAddress(d3d11Module, "D3D11CreateDeviceAndSwapChain");
        
        if (!original_create_device_and_swap_chain_) {
            LOG_ERROR("Failed to get D3D11CreateDeviceAndSwapChain address");
            return false;
        }

        LOG_INFO("DirectX hooks initialized successfully");
        return true;
    }

    void DXHook::Shutdown() {
        std::lock_guard<std::mutex> lock(hook_mutex_);
        
        RemoveHooks();
        
        original_create_device_and_swap_chain_ = nullptr;
        original_vs_set_shader_ = nullptr;
        original_ps_set_shader_ = nullptr;
        original_present_ = nullptr;
        device_context_vtable_ = nullptr;
        swap_chain_vtable_ = nullptr;
        hooked_device_ = nullptr;
        shader_manager_ = nullptr;
        
        LOG_INFO("DirectX hooks shut down");
    }

    bool DXHook::InstallHooks() {
        std::lock_guard<std::mutex> lock(hook_mutex_);
        
        if (hooks_installed_) {
            return true;
        }

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        // Hook D3D11CreateDeviceAndSwapChain
        DetourAttach(&(PVOID&)original_create_device_and_swap_chain_, HookedCreateDeviceAndSwapChain);
        
        LONG error = DetourTransactionCommit();
        if (error != NO_ERROR) {
            LOG_ERROR("Failed to install DirectX hooks: " + std::to_string(error));
            DetourTransactionAbort();
            return false;
        }

        hooks_installed_ = true;
        LOG_INFO("DirectX hooks installed successfully");
        return true;
    }

    bool DXHook::RemoveHooks() {
        if (!hooks_installed_) {
            return true;
        }

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        // Unhook functions
        if (original_create_device_and_swap_chain_) {
            DetourDetach(&(PVOID&)original_create_device_and_swap_chain_, HookedCreateDeviceAndSwapChain);
        }

        if (original_vs_set_shader_) {
            DetourDetach(&(PVOID&)original_vs_set_shader_, HookedVSSetShader);
        }

        if (original_ps_set_shader_) {
            DetourDetach(&(PVOID&)original_ps_set_shader_, HookedPSSetShader);
        }

        if (original_present_) {
            DetourDetach(&(PVOID&)original_present_, HookedPresent);
        }

        LONG error = DetourTransactionCommit();
        if (error != NO_ERROR) {
            LOG_ERROR("Failed to remove DirectX hooks: " + std::to_string(error));
            return false;
        }

        hooks_installed_ = false;
        LOG_INFO("DirectX hooks removed successfully");
        return true;
    }

    HRESULT WINAPI DXHook::HookedCreateDeviceAndSwapChain(
        IDXGIAdapter* pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags,
        const D3D_FEATURE_LEVEL* pFeatureLevels, UINT FeatureLevels, UINT SDKVersion,
        const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc, IDXGISwapChain** ppSwapChain,
        ID3D11Device** ppDevice, D3D_FEATURE_LEVEL* pFeatureLevel, ID3D11DeviceContext** ppImmediateContext) {
        
        LOG_DEBUG("D3D11CreateDeviceAndSwapChain called");
        
        // Call original function
        HRESULT hr = original_create_device_and_swap_chain_(
            pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, 
            SDKVersion, pSwapChainDesc, ppSwapChain, ppDevice, pFeatureLevel, ppImmediateContext
        );

        if (SUCCEEDED(hr) && ppDevice && *ppDevice && ppImmediateContext && *ppImmediateContext) {
            std::lock_guard<std::mutex> lock(hook_mutex_);
            
            hooked_device_ = *ppDevice;
            
            // Hook device context methods
            void** contextVTable = *(void***)(*ppImmediateContext);
            device_context_vtable_ = contextVTable;
            
            // Hook VSSetShader (index 11 in ID3D11DeviceContext vtable)
            original_vs_set_shader_ = (VSSetShader_t)contextVTable[11];
            HookVTable(contextVTable, 11, (void*)HookedVSSetShader, (void**)&original_vs_set_shader_);
            
            // Hook PSSetShader (index 9 in ID3D11DeviceContext vtable)  
            original_ps_set_shader_ = (PSSetShader_t)contextVTable[9];
            HookVTable(contextVTable, 9, (void*)HookedPSSetShader, (void**)&original_ps_set_shader_);

            // Hook swap chain Present if available
            if (ppSwapChain && *ppSwapChain) {
                void** swapChainVTable = *(void***)(*ppSwapChain);
                swap_chain_vtable_ = swapChainVTable;
                
                // Hook Present (index 8 in IDXGISwapChain vtable)
                original_present_ = (Present_t)swapChainVTable[8];
                HookVTable(swapChainVTable, 8, (void*)HookedPresent, (void**)&original_present_);
            }
            
            // Initialize shader manager
            InitializeShaderManager(*ppDevice);
            
            LOG_INFO("DirectX device and context hooks installed");
        }

        return hr;
    }

    void WINAPI DXHook::HookedVSSetShader(ID3D11DeviceContext* context, ID3D11VertexShader* shader,
                                        ID3D11ClassInstance* const* classInstances, UINT numClassInstances) {
        if (shader_manager_ && shader) {
            // Try to find RTX replacement
            // This would normally involve identifying the shader and replacing it
            // For now, we'll just pass through
        }

        // Call original function
        original_vs_set_shader_(context, shader, classInstances, numClassInstances);
    }

    void WINAPI DXHook::HookedPSSetShader(ID3D11DeviceContext* context, ID3D11PixelShader* shader,
                                        ID3D11ClassInstance* const* classInstances, UINT numClassInstances) {
        if (shader_manager_ && shader) {
            // Try to find RTX replacement
            // This would normally involve identifying the shader and replacing it
            // For now, we'll just pass through
        }

        // Call original function
        original_ps_set_shader_(context, shader, classInstances, numClassInstances);
    }

    HRESULT WINAPI DXHook::HookedPresent(IDXGISwapChain* swapChain, UINT syncInterval, UINT flags) {
        // This is called every frame - good place for per-frame operations
        
        // Call original function
        return original_present_(swapChain, syncInterval, flags);
    }

    bool DXHook::HookVTable(void** vtable, int index, void* newFunc, void** originalFunc) {
        if (!vtable || !newFunc) {
            return false;
        }

        DWORD oldProtect;
        if (!VirtualProtect(&vtable[index], sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtect)) {
            return false;
        }

        if (originalFunc) {
            *originalFunc = vtable[index];
        }
        vtable[index] = newFunc;

        VirtualProtect(&vtable[index], sizeof(void*), oldProtect, &oldProtect);
        return true;
    }

    void DXHook::UnhookVTable(void** vtable, int index, void* originalFunc) {
        if (!vtable || !originalFunc) {
            return;
        }

        DWORD oldProtect;
        if (VirtualProtect(&vtable[index], sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtect)) {
            vtable[index] = originalFunc;
            VirtualProtect(&vtable[index], sizeof(void*), oldProtect, &oldProtect);
        }
    }

    ID3D11Device* DXHook::GetDeviceFromContext(ID3D11DeviceContext* context) {
        ID3D11Device* device = nullptr;
        if (context) {
            context->GetDevice(&device);
        }
        return device;
    }

    void DXHook::InitializeShaderManager(ID3D11Device* device) {
        if (shader_manager_ && device) {
            shader_manager_->Initialize(device);
            LOG_INFO("Shader manager initialized with hooked device");
        }
    }
}
