#pragma once

#include "../common/types.h"
#include <commctrl.h>

namespace XShade {
    class SettingsDialog {
    public:
        SettingsDialog();
        ~SettingsDialog();

        bool Show(HWND parent = nullptr);
        
        void SetRTXSettings(const RTXSettings& settings);
        RTXSettings GetRTXSettings() const;
        
        // Callback for settings changes
        void SetSettingsChangedCallback(std::function<void(const RTXSettings&)> callback) {
            settings_changed_callback_ = callback;
        }

    private:
        static INT_PTR CALLBACK DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
        
        void InitializeControls(HWND hwnd);
        void UpdateControls(HWND hwnd);
        void SaveSettings(HWND hwnd);
        void LoadSettings();
        
        RTXSettings current_settings_;
        std::function<void(const RTXSettings&)> settings_changed_callback_;
        HWND dialog_hwnd_;
        
        // Control IDs
        enum ControlID {
            IDC_RTX_ENABLE = 1100,
            IDC_GI_INTENSITY = 1101,
            IDC_GI_INTENSITY_LABEL = 1102,
            IDC_REFLECTION_INTENSITY = 1103,
            IDC_REFLECTION_INTENSITY_LABEL = 1104,
            IDC_SHADOW_QUALITY = 1105,
            IDC_SHADOW_QUALITY_LABEL = 1106,
            IDC_BOUNCE_COUNT = 1107,
            IDC_BOUNCE_COUNT_LABEL = 1108,
            IDC_DENOISE_ENABLE = 1109,
            IDC_FEATURE_GI = 1110,
            IDC_FEATURE_REFLECTIONS = 1111,
            IDC_FEATURE_SHADOWS = 1112,
            IDC_FEATURE_AO = 1113,
            IDC_OK = 1114,
            IDC_CANCEL = 1115,
            IDC_RESET = 1116
        };
    };
}
