#pragma once

#include "../common/types.h"

namespace XShade {
    // Function pointer types for DirectX functions
    typedef HRESULT(WINAPI* D3D11CreateDeviceAndSwapChain_t)(
        IDXGIAdapter*, D3D_DRIVER_TYPE, HMODULE, UINT, 
        const D3D_FEATURE_LEVEL*, UINT, UINT, 
        const DXGI_SWAP_CHAIN_DESC*, IDXGISwapChain**, 
        ID3D11Device**, D3D_FEATURE_LEVEL*, ID3D11DeviceContext**
    );

    typedef void(WINAPI* VSSetShader_t)(ID3D11DeviceContext*, ID3D11VertexShader*, ID3D11ClassInstance* const*, UINT);
    typedef void(WINAPI* PSSetShader_t)(ID3D11DeviceContext*, ID3D11PixelShader*, ID3D11ClassInstance* const*, UINT);
    typedef void(WINAPI* Present_t)(IDXGISwapChain*, UINT, UINT);

    class DXHook {
    public:
        static bool Initialize();
        static void Shutdown();
        
        static bool InstallHooks();
        static bool RemoveHooks();
        
        static void SetShaderManager(ShaderManager* manager) { shader_manager_ = manager; }
        static void SetToggleCallback(ToggleCallback callback) { toggle_callback_ = callback; }
        
        // Hook functions
        static HRESULT WINAPI HookedCreateDeviceAndSwapChain(
            IDXGIAdapter* pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags,
            const D3D_FEATURE_LEVEL* pFeatureLevels, UINT FeatureLevels, UINT SDKVersion,
            const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc, IDXGISwapChain** ppSwapChain,
            ID3D11Device** ppDevice, D3D_FEATURE_LEVEL* pFeatureLevel, ID3D11DeviceContext** ppImmediateContext
        );

        static void WINAPI HookedVSSetShader(ID3D11DeviceContext* context, ID3D11VertexShader* shader,
                                           ID3D11ClassInstance* const* classInstances, UINT numClassInstances);

        static void WINAPI HookedPSSetShader(ID3D11DeviceContext* context, ID3D11PixelShader* shader,
                                           ID3D11ClassInstance* const* classInstances, UINT numClassInstances);

        static HRESULT WINAPI HookedPresent(IDXGISwapChain* swapChain, UINT syncInterval, UINT flags);

    private:
        static bool HookVTable(void** vtable, int index, void* newFunc, void** originalFunc);
        static void UnhookVTable(void** vtable, int index, void* originalFunc);
        
        static ID3D11Device* GetDeviceFromContext(ID3D11DeviceContext* context);
        static void InitializeShaderManager(ID3D11Device* device);

        // Original function pointers
        static D3D11CreateDeviceAndSwapChain_t original_create_device_and_swap_chain_;
        static VSSetShader_t original_vs_set_shader_;
        static PSSetShader_t original_ps_set_shader_;
        static Present_t original_present_;

        // Hook data
        static void** device_context_vtable_;
        static void** swap_chain_vtable_;
        static bool hooks_installed_;
        static ShaderManager* shader_manager_;
        static ToggleCallback toggle_callback_;
        static ID3D11Device* hooked_device_;
        static std::mutex hook_mutex_;
    };
}
