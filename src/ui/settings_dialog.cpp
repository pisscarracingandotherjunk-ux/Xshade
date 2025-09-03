#include "../include/ui/settings_dialog.h"
#include "../include/logging/logger.h"
#include "../include/config/configuration.h"

namespace XShade {
    SettingsDialog::SettingsDialog()
        : dialog_hwnd_(nullptr) {
        current_settings_ = Configuration::Instance().GetRTXSettings();
    }

    SettingsDialog::~SettingsDialog() {
    }

    bool SettingsDialog::Show(HWND parent) {
        // Create dialog template in memory (simplified approach)
        // In a real application, you'd typically use a resource file
        
        INT_PTR result = DialogBoxParamW(
            GetModuleHandle(nullptr),
            MAKEINTRESOURCEW(IDD_SETTINGS),
            parent,
            DialogProc,
            reinterpret_cast<LPARAM>(this)
        );
        
        return result == IDOK;
    }

    void SettingsDialog::SetRTXSettings(const RTXSettings& settings) {
        current_settings_ = settings;
    }

    RTXSettings SettingsDialog::GetRTXSettings() const {
        return current_settings_;
    }

    INT_PTR CALLBACK SettingsDialog::DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        SettingsDialog* dialog = reinterpret_cast<SettingsDialog*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));

        switch (uMsg) {
            case WM_INITDIALOG: {
                dialog = reinterpret_cast<SettingsDialog*>(lParam);
                SetWindowLongPtrW(hwnd, GWLP_USERDATA, lParam);
                dialog->dialog_hwnd_ = hwnd;
                dialog->InitializeControls(hwnd);
                return TRUE;
            }

            case WM_COMMAND:
                switch (LOWORD(wParam)) {
                    case IDOK:
                        if (dialog) {
                            dialog->SaveSettings(hwnd);
                        }
                        EndDialog(hwnd, IDOK);
                        return TRUE;

                    case IDCANCEL:
                        EndDialog(hwnd, IDCANCEL);
                        return TRUE;

                    case IDC_RESET:
                        if (dialog) {
                            // Reset to defaults
                            dialog->current_settings_ = RTXSettings{};
                            dialog->UpdateControls(hwnd);
                        }
                        return TRUE;

                    case IDC_RTX_ENABLE:
                        if (dialog && HIWORD(wParam) == BN_CLICKED) {
                            // Enable/disable other controls based on RTX enable state
                            BOOL enabled = IsDlgButtonChecked(hwnd, IDC_RTX_ENABLE) == BST_CHECKED;
                            EnableWindow(GetDlgItem(hwnd, IDC_GI_INTENSITY), enabled);
                            EnableWindow(GetDlgItem(hwnd, IDC_REFLECTION_INTENSITY), enabled);
                            EnableWindow(GetDlgItem(hwnd, IDC_SHADOW_QUALITY), enabled);
                            EnableWindow(GetDlgItem(hwnd, IDC_BOUNCE_COUNT), enabled);
                            EnableWindow(GetDlgItem(hwnd, IDC_DENOISE_ENABLE), enabled);
                            EnableWindow(GetDlgItem(hwnd, IDC_FEATURE_GI), enabled);
                            EnableWindow(GetDlgItem(hwnd, IDC_FEATURE_REFLECTIONS), enabled);
                            EnableWindow(GetDlgItem(hwnd, IDC_FEATURE_SHADOWS), enabled);
                            EnableWindow(GetDlgItem(hwnd, IDC_FEATURE_AO), enabled);
                        }
                        return TRUE;
                }
                break;

            case WM_HSCROLL:
                if (dialog) {
                    // Handle slider changes
                    HWND slider = reinterpret_cast<HWND>(lParam);
                    int position = static_cast<int>(SendMessage(slider, TBM_GETPOS, 0, 0));
                    
                    // Update corresponding label
                    int controlId = GetDlgCtrlID(slider);
                    wchar_t buffer[32];
                    
                    switch (controlId) {
                        case IDC_GI_INTENSITY:
                            swprintf_s(buffer, L"%.1f", position / 10.0f);
                            SetDlgItemTextW(hwnd, IDC_GI_INTENSITY_LABEL, buffer);
                            break;
                        case IDC_REFLECTION_INTENSITY:
                            swprintf_s(buffer, L"%.1f", position / 10.0f);
                            SetDlgItemTextW(hwnd, IDC_REFLECTION_INTENSITY_LABEL, buffer);
                            break;
                        case IDC_SHADOW_QUALITY:
                            swprintf_s(buffer, L"%.1f", position / 10.0f);
                            SetDlgItemTextW(hwnd, IDC_SHADOW_QUALITY_LABEL, buffer);
                            break;
                        case IDC_BOUNCE_COUNT:
                            swprintf_s(buffer, L"%d", position);
                            SetDlgItemTextW(hwnd, IDC_BOUNCE_COUNT_LABEL, buffer);
                            break;
                    }
                }
                return TRUE;
        }

        return FALSE;
    }

    void SettingsDialog::InitializeControls(HWND hwnd) {
        // Initialize slider controls
        SendDlgItemMessage(hwnd, IDC_GI_INTENSITY, TBM_SETRANGE, TRUE, MAKELPARAM(0, 30));
        SendDlgItemMessage(hwnd, IDC_GI_INTENSITY, TBM_SETTICFREQ, 5, 0);
        
        SendDlgItemMessage(hwnd, IDC_REFLECTION_INTENSITY, TBM_SETRANGE, TRUE, MAKELPARAM(0, 30));
        SendDlgItemMessage(hwnd, IDC_REFLECTION_INTENSITY, TBM_SETTICFREQ, 5, 0);
        
        SendDlgItemMessage(hwnd, IDC_SHADOW_QUALITY, TBM_SETRANGE, TRUE, MAKELPARAM(0, 30));
        SendDlgItemMessage(hwnd, IDC_SHADOW_QUALITY, TBM_SETTICFREQ, 5, 0);
        
        SendDlgItemMessage(hwnd, IDC_BOUNCE_COUNT, TBM_SETRANGE, TRUE, MAKELPARAM(1, 10));
        SendDlgItemMessage(hwnd, IDC_BOUNCE_COUNT, TBM_SETTICFREQ, 1, 0);

        UpdateControls(hwnd);
    }

    void SettingsDialog::UpdateControls(HWND hwnd) {
        // Update checkboxes
        CheckDlgButton(hwnd, IDC_RTX_ENABLE, current_settings_.enabled ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hwnd, IDC_DENOISE_ENABLE, current_settings_.denoise_enabled ? BST_CHECKED : BST_UNCHECKED);

        // Update feature checkboxes
        RTXFeature features = current_settings_.features;
        CheckDlgButton(hwnd, IDC_FEATURE_GI, 
                      (static_cast<int>(features) & static_cast<int>(RTXFeature::GlobalIllumination)) ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hwnd, IDC_FEATURE_REFLECTIONS,
                      (static_cast<int>(features) & static_cast<int>(RTXFeature::Reflections)) ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hwnd, IDC_FEATURE_SHADOWS,
                      (static_cast<int>(features) & static_cast<int>(RTXFeature::Shadows)) ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hwnd, IDC_FEATURE_AO,
                      (static_cast<int>(features) & static_cast<int>(RTXFeature::AmbientOcclusion)) ? BST_CHECKED : BST_UNCHECKED);

        // Update sliders
        SendDlgItemMessage(hwnd, IDC_GI_INTENSITY, TBM_SETPOS, TRUE, 
                          static_cast<LPARAM>(current_settings_.global_illumination_intensity * 10));
        SendDlgItemMessage(hwnd, IDC_REFLECTION_INTENSITY, TBM_SETPOS, TRUE,
                          static_cast<LPARAM>(current_settings_.reflection_intensity * 10));
        SendDlgItemMessage(hwnd, IDC_SHADOW_QUALITY, TBM_SETPOS, TRUE,
                          static_cast<LPARAM>(current_settings_.shadow_quality * 10));
        SendDlgItemMessage(hwnd, IDC_BOUNCE_COUNT, TBM_SETPOS, TRUE, current_settings_.bounce_count);

        // Update labels
        wchar_t buffer[32];
        swprintf_s(buffer, L"%.1f", current_settings_.global_illumination_intensity);
        SetDlgItemTextW(hwnd, IDC_GI_INTENSITY_LABEL, buffer);
        
        swprintf_s(buffer, L"%.1f", current_settings_.reflection_intensity);
        SetDlgItemTextW(hwnd, IDC_REFLECTION_INTENSITY_LABEL, buffer);
        
        swprintf_s(buffer, L"%.1f", current_settings_.shadow_quality);
        SetDlgItemTextW(hwnd, IDC_SHADOW_QUALITY_LABEL, buffer);
        
        swprintf_s(buffer, L"%d", current_settings_.bounce_count);
        SetDlgItemTextW(hwnd, IDC_BOUNCE_COUNT_LABEL, buffer);

        // Enable/disable controls based on RTX enable state
        BOOL enabled = current_settings_.enabled;
        EnableWindow(GetDlgItem(hwnd, IDC_GI_INTENSITY), enabled);
        EnableWindow(GetDlgItem(hwnd, IDC_REFLECTION_INTENSITY), enabled);
        EnableWindow(GetDlgItem(hwnd, IDC_SHADOW_QUALITY), enabled);
        EnableWindow(GetDlgItem(hwnd, IDC_BOUNCE_COUNT), enabled);
        EnableWindow(GetDlgItem(hwnd, IDC_DENOISE_ENABLE), enabled);
        EnableWindow(GetDlgItem(hwnd, IDC_FEATURE_GI), enabled);
        EnableWindow(GetDlgItem(hwnd, IDC_FEATURE_REFLECTIONS), enabled);
        EnableWindow(GetDlgItem(hwnd, IDC_FEATURE_SHADOWS), enabled);
        EnableWindow(GetDlgItem(hwnd, IDC_FEATURE_AO), enabled);
    }

    void SettingsDialog::SaveSettings(HWND hwnd) {
        // Read values from controls
        current_settings_.enabled = IsDlgButtonChecked(hwnd, IDC_RTX_ENABLE) == BST_CHECKED;
        current_settings_.denoise_enabled = IsDlgButtonChecked(hwnd, IDC_DENOISE_ENABLE) == BST_CHECKED;

        // Read feature flags
        int features = 0;
        if (IsDlgButtonChecked(hwnd, IDC_FEATURE_GI) == BST_CHECKED)
            features |= static_cast<int>(RTXFeature::GlobalIllumination);
        if (IsDlgButtonChecked(hwnd, IDC_FEATURE_REFLECTIONS) == BST_CHECKED)
            features |= static_cast<int>(RTXFeature::Reflections);
        if (IsDlgButtonChecked(hwnd, IDC_FEATURE_SHADOWS) == BST_CHECKED)
            features |= static_cast<int>(RTXFeature::Shadows);
        if (IsDlgButtonChecked(hwnd, IDC_FEATURE_AO) == BST_CHECKED)
            features |= static_cast<int>(RTXFeature::AmbientOcclusion);
        current_settings_.features = static_cast<RTXFeature>(features);

        // Read slider values
        current_settings_.global_illumination_intensity = 
            SendDlgItemMessage(hwnd, IDC_GI_INTENSITY, TBM_GETPOS, 0, 0) / 10.0f;
        current_settings_.reflection_intensity = 
            SendDlgItemMessage(hwnd, IDC_REFLECTION_INTENSITY, TBM_GETPOS, 0, 0) / 10.0f;
        current_settings_.shadow_quality = 
            SendDlgItemMessage(hwnd, IDC_SHADOW_QUALITY, TBM_GETPOS, 0, 0) / 10.0f;
        current_settings_.bounce_count = 
            static_cast<int>(SendDlgItemMessage(hwnd, IDC_BOUNCE_COUNT, TBM_GETPOS, 0, 0));

        // Save to configuration
        Configuration::Instance().SetRTXSettings(current_settings_);
        Configuration::Instance().Save();

        // Notify callback
        if (settings_changed_callback_) {
            settings_changed_callback_(current_settings_);
        }

        LOG_INFO("Settings saved successfully");
    }

    void SettingsDialog::LoadSettings() {
        current_settings_ = Configuration::Instance().GetRTXSettings();
    }
}
