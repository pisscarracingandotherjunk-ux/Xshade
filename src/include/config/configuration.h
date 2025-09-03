#pragma once

#include "../common/types.h"

namespace XShade {
    class Configuration {
    public:
        static Configuration& Instance();
        
        bool Load(const std::string& configFile = CONFIG_FILE);
        bool Save(const std::string& configFile = CONFIG_FILE);
        
        // RTX Settings
        RTXSettings& GetRTXSettings() { return rtx_settings_; }
        void SetRTXSettings(const RTXSettings& settings) { rtx_settings_ = settings; }
        
        // Hotkeys
        int GetToggleHotkey() const { return toggle_hotkey_; }
        void SetToggleHotkey(int hotkey) { toggle_hotkey_ = hotkey; }
        
        // General Settings
        bool IsAutoStartEnabled() const { return auto_start_; }
        void SetAutoStart(bool enabled) { auto_start_ = enabled; }
        
        bool IsMinimizeToTrayEnabled() const { return minimize_to_tray_; }
        void SetMinimizeToTray(bool enabled) { minimize_to_tray_ = enabled; }
        
        LogLevel GetLogLevel() const { return log_level_; }
        void SetLogLevel(LogLevel level) { log_level_ = level; }

    private:
        Configuration() = default;
        ~Configuration() = default;
        Configuration(const Configuration&) = delete;
        Configuration& operator=(const Configuration&) = delete;
        
        void SetDefaultValues();
        std::string ReadValue(const std::string& section, const std::string& key, 
                             const std::string& defaultValue, const std::string& configFile);
        void WriteValue(const std::string& section, const std::string& key, 
                       const std::string& value, const std::string& configFile);

        RTXSettings rtx_settings_;
        int toggle_hotkey_ = VK_F10;  // F10 by default
        bool auto_start_ = false;
        bool minimize_to_tray_ = true;
        LogLevel log_level_ = LogLevel::Info;
    };
}
