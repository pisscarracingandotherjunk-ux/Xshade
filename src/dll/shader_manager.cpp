#include "../include/dll/shader_manager.h"
#include "../include/dll/shader_compiler.h"
#include "../include/logging/logger.h"
#include <filesystem>

namespace XShade {
    ShaderManager::ShaderManager()
        : device_(nullptr)
        , context_(nullptr)
        , current_mode_(RenderMode::Vanilla) {
    }

    ShaderManager::~ShaderManager() {
        Shutdown();
    }

    bool ShaderManager::Initialize(ID3D11Device* device) {
        if (!device) {
            LOG_ERROR("Invalid device provided to ShaderManager");
            return false;
        }
        
        device_ = device;
        device_->GetImmediateContext(&context_);
        
        LOG_INFO("ShaderManager initialized");
        return LoadShaders();
    }

    void ShaderManager::Shutdown() {
        std::lock_guard<std::mutex> lock(shader_mutex_);
        
        // Release all shader resources
        for (auto& [name, shader] : vanilla_shaders_) {
            SAFE_RELEASE(shader->shader);
        }
        vanilla_shaders_.clear();
        
        for (auto& [name, shader] : rtx_shaders_) {
            SAFE_RELEASE(shader->shader);
        }
        rtx_shaders_.clear();
        
        SAFE_RELEASE(context_);
        device_ = nullptr;
        
        LOG_INFO("ShaderManager shut down");
    }

    bool ShaderManager::LoadShaders() {
        LOG_INFO("Loading shaders...");
        
        bool success = true;
        success &= LoadVanillaShaders();
        success &= LoadRTXShaders();
        
        if (success) {
            LOG_INFO("All shaders loaded successfully");
        } else {
            LOG_ERROR("Some shaders failed to load");
        }
        
        return success;
    }

    bool ShaderManager::CompileShader(const std::string& source, const std::string& entryPoint, 
                                     const std::string& target, ID3DBlob** blob) {
        return ShaderCompiler::CompileShaderFromSource(source, entryPoint, target, blob);
    }

    ShaderPtr ShaderManager::GetShader(const std::string& name) {
        std::lock_guard<std::mutex> lock(shader_mutex_);
        
        if (current_mode_ == RenderMode::RTX) {
            auto it = rtx_shaders_.find(name);
            if (it != rtx_shaders_.end()) {
                return it->second;
            }
        }
        
        auto it = vanilla_shaders_.find(name);
        if (it != vanilla_shaders_.end()) {
            return it->second;
        }
        
        return nullptr;
    }

    bool ShaderManager::RegisterShader(const std::string& name, ShaderPtr shader) {
        std::lock_guard<std::mutex> lock(shader_mutex_);
        
        if (current_mode_ == RenderMode::RTX) {
            rtx_shaders_[name] = shader;
        } else {
            vanilla_shaders_[name] = shader;
        }
        
        LOG_DEBUG("Registered shader: " + name);
        return true;
    }

    bool ShaderManager::CreateRTXVariant(ShaderPtr originalShader) {
        if (!originalShader) {
            return false;
        }
        
        // Create RTX-enhanced version of the shader
        auto rtxShader = std::make_shared<ShaderInfo>();
        rtxShader->name = originalShader->name + "_rtx";
        rtxShader->type = originalShader->type;
        rtxShader->source = originalShader->source; // Would be modified for RTX features
        
        // Compile RTX version
        ID3DBlob* shaderBlob = nullptr;
        ID3DBlob* errorBlob = nullptr;
        
        std::string target = ShaderCompiler::GetTargetFromShaderType(rtxShader->type);
        bool compiled = ShaderCompiler::CompileShaderFromSource(
            rtxShader->source, "main", target, &shaderBlob, &errorBlob
        );
        
        if (!compiled) {
            SAFE_RELEASE(errorBlob);
            return false;
        }
        
        // Create D3D shader object
        bool created = false;
        switch (rtxShader->type) {
            case ShaderType::Vertex: {
                ID3D11VertexShader* vs = nullptr;
                created = ShaderCompiler::CreateVertexShader(device_, shaderBlob, &vs);
                rtxShader->shader = vs;
                break;
            }
            case ShaderType::Pixel: {
                ID3D11PixelShader* ps = nullptr;
                created = ShaderCompiler::CreatePixelShader(device_, shaderBlob, &ps);
                rtxShader->shader = ps;
                break;
            }
            case ShaderType::Compute: {
                ID3D11ComputeShader* cs = nullptr;
                created = ShaderCompiler::CreateComputeShader(device_, shaderBlob, &cs);
                rtxShader->shader = cs;
                break;
            }
        }
        
        SAFE_RELEASE(shaderBlob);
        SAFE_RELEASE(errorBlob);
        
        if (created) {
            rtx_shaders_[originalShader->name] = rtxShader;
            originalShader->rtx_shader = rtxShader->shader;
            return true;
        }
        
        return false;
    }

    ID3D11DeviceChild* ShaderManager::GetActiveShader(const std::string& name, RenderMode mode) {
        std::lock_guard<std::mutex> lock(shader_mutex_);
        
        if (mode == RenderMode::RTX) {
            auto it = rtx_shaders_.find(name);
            if (it != rtx_shaders_.end()) {
                return it->second->shader;
            }
        }
        
        auto it = vanilla_shaders_.find(name);
        if (it != vanilla_shaders_.end()) {
            return it->second->shader;
        }
        
        return nullptr;
    }

    void ShaderManager::SetRenderMode(RenderMode mode) {
        current_mode_ = mode;
        LOG_INFO(std::string("Render mode changed to: ") + 
                (mode == RenderMode::RTX ? "RTX" : "Vanilla"));
    }

    bool ShaderManager::LoadShaderFromFile(const std::string& filePath) {
        if (!std::filesystem::exists(filePath)) {
            LOG_ERROR("Shader file not found: " + filePath);
            return false;
        }
        
        auto shader = std::make_shared<ShaderInfo>();
        
        // Extract name from file path
        std::filesystem::path path(filePath);
        shader->name = path.stem().string();
        
        // Determine shader type from filename
        shader->type = GetShaderTypeFromFilename(shader->name);
        
        // Compile shader
        ID3DBlob* shaderBlob = nullptr;
        ID3DBlob* errorBlob = nullptr;
        
        std::string target = ShaderCompiler::GetTargetFromShaderType(shader->type);
        bool compiled = ShaderCompiler::CompileShaderFromFile(
            filePath, "main", target, &shaderBlob, &errorBlob
        );
        
        if (!compiled) {
            SAFE_RELEASE(errorBlob);
            return false;
        }
        
        // Store bytecode
        shader->bytecode.resize(shaderBlob->GetBufferSize());
        memcpy(shader->bytecode.data(), shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize());
        
        // Create D3D shader object
        bool created = false;
        switch (shader->type) {
            case ShaderType::Vertex: {
                ID3D11VertexShader* vs = nullptr;
                created = ShaderCompiler::CreateVertexShader(device_, shaderBlob, &vs);
                shader->shader = vs;
                break;
            }
            case ShaderType::Pixel: {
                ID3D11PixelShader* ps = nullptr;
                created = ShaderCompiler::CreatePixelShader(device_, shaderBlob, &ps);
                shader->shader = ps;
                break;
            }
        }
        
        SAFE_RELEASE(shaderBlob);
        SAFE_RELEASE(errorBlob);
        
        if (created) {
            RegisterShader(shader->name, shader);
            return true;
        }
        
        return false;
    }

    bool ShaderManager::LoadVanillaShaders() {
        std::string shaderDir = GetShaderPath("");
        
        bool success = true;
        success &= LoadShaderFromFile(GetShaderPath("vanilla_vertex.hlsl"));
        success &= LoadShaderFromFile(GetShaderPath("vanilla_pixel.hlsl"));
        
        return success;
    }

    bool ShaderManager::LoadRTXShaders() {
        std::string shaderDir = GetShaderPath("");
        
        bool success = true;
        success &= LoadShaderFromFile(GetShaderPath("rtx_vertex.hlsl"));
        success &= LoadShaderFromFile(GetShaderPath("rtx_pixel.hlsl"));
        
        return success;
    }

    std::string ShaderManager::GetShaderPath(const std::string& filename) {
        char modulePath[MAX_PATH];
        GetModuleFileNameA(nullptr, modulePath, MAX_PATH);
        std::filesystem::path exePath(modulePath);
        return (exePath.parent_path() / "shaders" / filename).string();
    }

    ShaderType ShaderManager::GetShaderTypeFromFilename(const std::string& filename) {
        if (filename.find("vertex") != std::string::npos) return ShaderType::Vertex;
        if (filename.find("pixel") != std::string::npos) return ShaderType::Pixel;
        if (filename.find("compute") != std::string::npos) return ShaderType::Compute;
        if (filename.find("geometry") != std::string::npos) return ShaderType::Geometry;
        if (filename.find("hull") != std::string::npos) return ShaderType::Hull;
        if (filename.find("domain") != std::string::npos) return ShaderType::Domain;
        
        return ShaderType::Vertex; // Default
    }
}
