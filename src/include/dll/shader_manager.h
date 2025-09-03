#pragma once

#include "../common/types.h"

namespace XShade {
    class ShaderManager {
    public:
        ShaderManager();
        ~ShaderManager();

        bool Initialize(ID3D11Device* device);
        void Shutdown();

        bool LoadShaders();
        bool CompileShader(const std::string& source, const std::string& entryPoint, 
                          const std::string& target, ID3DBlob** blob);
        
        ShaderPtr GetShader(const std::string& name);
        bool RegisterShader(const std::string& name, ShaderPtr shader);
        
        bool CreateRTXVariant(ShaderPtr originalShader);
        ID3D11DeviceChild* GetActiveShader(const std::string& name, RenderMode mode);
        
        void SetRenderMode(RenderMode mode);
        RenderMode GetRenderMode() const { return current_mode_; }

    private:
        bool LoadShaderFromFile(const std::string& filePath);
        bool LoadVanillaShaders();
        bool LoadRTXShaders();
        
        std::string GetShaderPath(const std::string& filename);
        ShaderType GetShaderTypeFromFilename(const std::string& filename);

        ID3D11Device* device_;
        ID3D11DeviceContext* context_;
        ShaderMap vanilla_shaders_;
        ShaderMap rtx_shaders_;
        std::atomic<RenderMode> current_mode_;
        std::mutex shader_mutex_;
    };
}
