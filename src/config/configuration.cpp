#include "../include/config/configuration.h"
#include "../include/logging/logger.h"
#include <windows.h>
#include <fstream>
#include <sstream>

namespace XShade {
    Configuration& Configuration::Instance() {
        static Configuration instance;
        return instance;
    }

    bool Configuration::Load(const std::string& configFile) {
        SetDefaultValues();
        
        std::ifstream file(configFile);
        if (!file.is_open()) {
            LOG_INFO("Config file not found, using defaults: " + configFile);
            Save(configFile);  // Create with defaults
            return true;
        }

        try {
            // RTX Settings
            rtx_settings_.enabled = (ReadValue("RTX", "Enabled", "0", configFile) == "1");
            rtx_settings_.global_illumination_intensity = 
                std::stof(ReadValue("RTX", "GlobalIlluminationIntensity", "1.0", configFile));
            rtx_settings_.reflection_intensity = 
                std::stof(ReadValue("RTX", "ReflectionIntensity", "1.0", configFile));
            rtx_settings_.shadow_quality = 
                std::stof(ReadValue("RTX", "ShadowQuality", "1.0", configFile));
            rtx_settings_.bounce_count = 
                std::stoi(ReadValue("RTX", "BounceCount", "3", configFile));
            rtx_settings_.denoise_enabled = 
                (ReadValue("RTX", "DenoiseEnabled", "1", configFile) == "1");

            // Features bitmask
            int features = std::stoi(ReadValue("RTX", "Features", 
                std::to_string(static_cast<int>(RTXFeature::All)), configFile));
            rtx_settings_.features = static_cast<RTXFeature>(features);

            // General Settings
            toggle_hotkey_ = std::stoi(ReadValue("General", "ToggleHotkey", 
                std::to_string(VK_F10), configFile));
            auto_start_ = (ReadValue("General", "AutoStart", "0", configFile) == "1");
            minimize_to_tray_ = (ReadValue("General", "MinimizeToTray", "1", configFile) == "1");
            
            int logLevel = std::stoi(ReadValue("General", "LogLevel", "1", configFile));
            log_level_ = static_cast<LogLevel>(logLevel);

            LOG_INFO("Configuration loaded successfully");
            return true;
        }
        catch (const std::exception& e) {
            LOG_ERROR("Error loading configuration: " + std::string(e.what()));
            SetDefaultValues();
            return false;
        }
    }

    bool Configuration::Save(const std::string& configFile) {
        try {
            std::ofstream file(configFile);
            if (!file.is_open()) {
                LOG_ERROR("Failed to open config file for writing: " + configFile);
                return false;
            }

            file << "# XShade Configuration File\n";
            file << "# Generated automatically - modify with care\n\n";

            // RTX Section
            file << "[RTX]\n";
            file << "Enabled=" << (rtx_settings_.enabled ? "1" : "0") << "\n";
            file << "Features=" << static_cast<int>(rtx_settings_.features) << "\n";
            file << "GlobalIlluminationIntensity=" << rtx_settings_.global_illumination_intensity << "\n";
            file << "ReflectionIntensity=" << rtx_settings_.reflection_intensity << "\n";
            file << "ShadowQuality=" << rtx_settings_.shadow_quality << "\n";
            file << "BounceCount=" << rtx_settings_.bounce_count << "\n";
            file << "DenoiseEnabled=" << (rtx_settings_.denoise_enabled ? "1" : "0") << "\n\n";

            // General Section
            file << "[General]\n";
            file << "ToggleHotkey=" << toggle_hotkey_ << "\n";
            file << "AutoStart=" << (auto_start_ ? "1" : "0") << "\n";
            file << "MinimizeToTray=" << (minimize_to_tray_ ? "1" : "0") << "\n";
            file << "LogLevel=" << static_cast<int>(log_level_) << "\n";

            file.close();
            LOG_INFO("Configuration saved successfully");
            return true;
        }
        catch (const std::exception& e) {
            LOG_ERROR("Error saving configuration: " + std::string(e.what()));
            return false;
        }
    }

    void Configuration::SetDefaultValues() {
        rtx_settings_ = RTXSettings{};
        rtx_settings_.enabled = false;
        rtx_settings_.features = RTXFeature::All;
        rtx_settings_.global_illumination_intensity = 1.0f;
        rtx_settings_.reflection_intensity = 1.0f;
        rtx_settings_.shadow_quality = 1.0f;
        rtx_settings_.bounce_count = 3;
        rtx_settings_.denoise_enabled = true;

        toggle_hotkey_ = VK_F10;
        auto_start_ = false;
        minimize_to_tray_ = true;
        log_level_ = LogLevel::Info;
    }

    std::string Configuration::ReadValue(const std::string& section, const std::string& key,
                                       const std::string& defaultValue, const std::string& configFile) {
        char buffer[256];
        DWORD result = GetPrivateProfileStringA(
            section.c_str(),
            key.c_str(),
            defaultValue.c_str(),
            buffer,
            sizeof(buffer),
            configFile.c_str()
        );
        
        return std::string(buffer);
    }

    void Configuration::WriteValue(const std::string& section, const std::string& key,
                                 const std::string& value, const std::string& configFile) {
        WritePrivateProfileStringA(
            section.c_str(),
            key.c_str(),
            value.c_str(),
            configFile.c_str()
        );
    }
}
