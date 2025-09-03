#pragma once

#include "../common/types.h"

namespace XShade {
    class ShaderCompiler {
    public:
        static bool CompileShaderFromFile(const std::string& filePath, 
                                        const std::string& entryPoint,
                                        const std::string& target,
                                        ID3DBlob** shaderBlob,
                                        ID3DBlob** errorBlob = nullptr);
        
        static bool CompileShaderFromSource(const std::string& source,
                                          const std::string& entryPoint,
                                          const std::string& target,
                                          ID3DBlob** shaderBlob,
                                          ID3DBlob** errorBlob = nullptr);
        
        static bool CreateVertexShader(ID3D11Device* device, 
                                     ID3DBlob* shaderBlob,
                                     ID3D11VertexShader** shader);
        
        static bool CreatePixelShader(ID3D11Device* device,
                                    ID3DBlob* shaderBlob,
                                    ID3D11PixelShader** shader);
        
        static bool CreateComputeShader(ID3D11Device* device,
                                      ID3DBlob* shaderBlob,
                                      ID3D11ComputeShader** shader);
        
        static ShaderType GetShaderTypeFromTarget(const std::string& target);
        static std::string GetTargetFromShaderType(ShaderType type);
        
    private:
        static std::string ReadFileContents(const std::string& filePath);
        static void SetCompilerFlags(DWORD& flags);
    };
}
