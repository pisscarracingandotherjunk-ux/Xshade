#include "../include/dll/shader_compiler.h"
#include "../include/logging/logger.h"
#include <fstream>
#include <sstream>

namespace XShade {
    bool ShaderCompiler::CompileShaderFromFile(const std::string& filePath, 
                                             const std::string& entryPoint,
                                             const std::string& target,
                                             ID3DBlob** shaderBlob,
                                             ID3DBlob** errorBlob) {
        std::string source = ReadFileContents(filePath);
        if (source.empty()) {
            LOG_ERROR("Failed to read shader file: " + filePath);
            return false;
        }
        
        return CompileShaderFromSource(source, entryPoint, target, shaderBlob, errorBlob);
    }

    bool ShaderCompiler::CompileShaderFromSource(const std::string& source,
                                               const std::string& entryPoint,
                                               const std::string& target,
                                               ID3DBlob** shaderBlob,
                                               ID3DBlob** errorBlob) {
        DWORD flags = 0;
        SetCompilerFlags(flags);
        
        HRESULT hr = D3DCompile(
            source.c_str(),
            source.length(),
            nullptr,
            nullptr,
            nullptr,
            entryPoint.c_str(),
            target.c_str(),
            flags,
            0,
            shaderBlob,
            errorBlob
        );
        
        if (FAILED(hr)) {
            std::string errorMsg = "Shader compilation failed for target: " + target;
            
            if (errorBlob && *errorBlob) {
                char* errorStr = (char*)(*errorBlob)->GetBufferPointer();
                errorMsg += "\nError: " + std::string(errorStr);
            }
            
            LOG_ERROR(errorMsg);
            return false;
        }
        
        LOG_DEBUG("Successfully compiled shader: " + target + "::" + entryPoint);
        return true;
    }

    bool ShaderCompiler::CreateVertexShader(ID3D11Device* device, 
                                           ID3DBlob* shaderBlob,
                                           ID3D11VertexShader** shader) {
        HRESULT hr = device->CreateVertexShader(
            shaderBlob->GetBufferPointer(),
            shaderBlob->GetBufferSize(),
            nullptr,
            shader
        );
        
        if (FAILED(hr)) {
            LOG_ERROR("Failed to create vertex shader: " + std::to_string(hr));
            return false;
        }
        
        return true;
    }

    bool ShaderCompiler::CreatePixelShader(ID3D11Device* device,
                                          ID3DBlob* shaderBlob,
                                          ID3D11PixelShader** shader) {
        HRESULT hr = device->CreatePixelShader(
            shaderBlob->GetBufferPointer(),
            shaderBlob->GetBufferSize(),
            nullptr,
            shader
        );
        
        if (FAILED(hr)) {
            LOG_ERROR("Failed to create pixel shader: " + std::to_string(hr));
            return false;
        }
        
        return true;
    }

    bool ShaderCompiler::CreateComputeShader(ID3D11Device* device,
                                            ID3DBlob* shaderBlob,
                                            ID3D11ComputeShader** shader) {
        HRESULT hr = device->CreateComputeShader(
            shaderBlob->GetBufferPointer(),
            shaderBlob->GetBufferSize(),
            nullptr,
            shader
        );
        
        if (FAILED(hr)) {
            LOG_ERROR("Failed to create compute shader: " + std::to_string(hr));
            return false;
        }
        
        return true;
    }

    ShaderType ShaderCompiler::GetShaderTypeFromTarget(const std::string& target) {
        if (target.find("vs_") == 0) return ShaderType::Vertex;
        if (target.find("ps_") == 0) return ShaderType::Pixel;
        if (target.find("cs_") == 0) return ShaderType::Compute;
        if (target.find("gs_") == 0) return ShaderType::Geometry;
        if (target.find("hs_") == 0) return ShaderType::Hull;
        if (target.find("ds_") == 0) return ShaderType::Domain;
        
        return ShaderType::Vertex; // Default
    }

    std::string ShaderCompiler::GetTargetFromShaderType(ShaderType type) {
        switch (type) {
            case ShaderType::Vertex:   return "vs_5_0";
            case ShaderType::Pixel:    return "ps_5_0";
            case ShaderType::Compute:  return "cs_5_0";
            case ShaderType::Geometry: return "gs_5_0";
            case ShaderType::Hull:     return "hs_5_0";
            case ShaderType::Domain:   return "ds_5_0";
            default:                   return "vs_5_0";
        }
    }

    std::string ShaderCompiler::ReadFileContents(const std::string& filePath) {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            return "";
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    void ShaderCompiler::SetCompilerFlags(DWORD& flags) {
#ifdef _DEBUG
        flags |= D3DCOMPILE_DEBUG;
        flags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#else
        flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif
        
        flags |= D3DCOMPILE_ENABLE_STRICTNESS;
        flags |= D3DCOMPILE_WARNINGS_ARE_ERRORS;
    }
}
