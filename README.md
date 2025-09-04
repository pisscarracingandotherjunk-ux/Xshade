# XShade - RTX Enhancement for Roblox

XShade is a Windows 11 x64 application that enhances Roblox graphics by replacing its vanilla shader system with RTX-enabled shaders, providing global illumination, real-time reflections, enhanced shadows, and ambient occlusion.

## Features

- **Global RTX Integration**: Seamlessly replaces Roblox's shader API with RTX-enhanced alternatives
- **Toggleable Rendering**: Switch between vanilla and RTX modes on-the-fly (F10 hotkey)
- **Advanced RTX Features**:
  - Global Illumination with configurable intensity
  - Real-time reflections and screen-space reflections
  - Raytraced shadows with soft shadow support
  - Screen-space ambient occlusion
  - Temporal denoising for smooth results
- **System Tray Integration**: Convenient system tray icon with context menu
- **Settings GUI**: Comprehensive settings dialog for fine-tuning RTX parameters
- **Process Monitoring**: Automatically detects Roblox launches and injects enhancements
- **Configuration Persistence**: Settings are saved and restored between sessions

## Technical Architecture

### Core Components

1. **XShade.exe** - Main application with system tray and process monitoring
2. **XShadeCore.dll** - Injectable DLL that hooks DirectX API calls
3. **RTX Shaders** - HLSL shaders implementing RTX features
4. **Configuration System** - INI-based settings management

### How It Works

1. **Process Monitoring**: Continuously monitors for RobloxPlayerBeta.exe
2. **DLL Injection**: When Roblox launches, injects XShadeCore.dll using standard Windows API
3. **API Hooking**: Intercepts DirectX 11 calls (CreateDeviceAndSwapChain, VSSetShader, PSSetShader)
4. **Shader Replacement**: Replaces vanilla shaders with RTX-enhanced versions in real-time
5. **Render Mode Toggle**: Switches between shader sets based on user preference

## Build Requirements

- Windows 11 x64
- Visual Studio 2022 (Community Edition or higher)
- Windows 10/11 SDK
- CMake 3.20+
- DirectX 11 SDK (included with Windows SDK)

## Building

### Quick Start

1. Clone the repository
2. Run `setup.bat` to install prerequisites (optional but recommended)
3. Open Command Prompt as Administrator
4. Run `build.bat`
5. Executable will be created in `build\bin\Release\`

### Prerequisites

If you encounter build errors, ensure you have:
- CMake 3.20+ ([download here](https://cmake.org/download/))
- Visual Studio 2022 with C++ development tools ([download here](https://visualstudio.microsoft.com/downloads/))
- Windows 10/11 SDK

### Manual Build Steps

```bash
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

### Automated Setup

Run `setup.bat` to automatically check and install missing prerequisites:
- Detects if CMake is installed
- Offers multiple installation options (winget, chocolatey, manual)
- Checks for Visual Studio 2022
- Provides clear guidance for missing components

## Installation & Usage

1. **Run as Administrator**: XShade requires admin privileges for DLL injection
2. **Launch XShade.exe**: The application runs in the system tray
3. **Launch Roblox**: XShade will automatically detect and enhance Roblox
4. **Toggle RTX**: Use F10 or right-click the tray icon to toggle RTX mode
5. **Configure Settings**: Right-click tray icon → Settings for advanced options

## Configuration

Settings are stored in `xshade.ini`:

```ini
[RTX]
Enabled=0                        # Enable RTX by default
Features=15                      # Feature bitmask (15 = all features)
GlobalIlluminationIntensity=1.0  # GI strength (0.0-3.0)
ReflectionIntensity=1.0         # Reflection strength (0.0-3.0)
ShadowQuality=1.0               # Shadow quality (0.0-3.0)
BounceCount=3                   # Ray bounce count (1-10)
DenoiseEnabled=1                # Enable temporal denoising

[General]
ToggleHotkey=121                # F10 key code
AutoStart=0                     # Start with Windows
MinimizeToTray=1                # Minimize to system tray
LogLevel=1                      # Logging verbosity
```

## RTX Features Explained

### Global Illumination
- Simulates realistic light bouncing between surfaces
- Provides natural ambient lighting and color bleeding
- Configurable intensity and bounce count

### Real-time Reflections  
- Screen-space reflections for visible surfaces
- Environment map fallback for off-screen reflections
- Fresnel-accurate reflection intensity

### Enhanced Shadows
- Raytraced soft shadows with configurable softness
- Multiple shadow samples for realistic penumbra
- Temporal stability for smooth shadow edges

### Ambient Occlusion
- Screen-space ambient occlusion for depth perception
- Contact shadows in crevices and corners
- Configurable intensity and radius

## Compatibility

- **Platform**: Windows 11 x64 only
- **Roblox Version**: Compatible with current RobloxPlayerBeta.exe
- **Graphics**: DirectX 11 compatible GPU (RTX GPUs recommended)
- **Memory**: 4GB+ RAM recommended
- **Storage**: 50MB disk space

## Safety & Anti-Cheat

XShade is designed as a graphics enhancement tool and:
- Only modifies rendering shaders, not game logic
- Does not alter game mechanics or provide gameplay advantages  
- Operates at the DirectX API level, similar to ReShade
- May be detected by anti-cheat systems (use at your own discretion)

## Troubleshooting

### Common Issues

**"cmake is not recognized" error when running build.bat**
- CMake is not installed or not in system PATH
- Run `setup.bat` for automated installation
- Or manually install from https://cmake.org/download/
- During installation, check "Add CMake to system PATH"
- Restart command prompt after installation

**XShade won't start**
- Ensure running as Administrator
- Check Windows Defender exclusions
- Verify Visual C++ 2022 Redistributable is installed

**Roblox not detected**
- Make sure Roblox is running
- Check antivirus isn't blocking DLL injection
- Restart XShade after launching Roblox

**RTX features not working**
- Verify DirectX 11 compatible GPU
- Check Roblox is using DX11 renderer
- Review xshade.log for error messages

**Performance issues**
- Lower RTX settings (bounce count, shadow quality)
- Disable expensive features (global illumination)
- Ensure adequate GPU memory available

### Log Files

Check these files for diagnostic information:
- `xshade.log` - Main application log
- `xshade_core.log` - DLL injection and hooking log

## Legal Notice

This software is provided for educational purposes. Users are responsible for compliance with Roblox Terms of Service and any applicable anti-cheat policies. The developers are not responsible for any account restrictions that may result from using this software.

## License

MIT License - see LICENSE file for details.

## Contributing

Contributions welcome! Please read contributing guidelines and submit pull requests for any enhancements.

---

**Disclaimer**: This is an unofficial third-party tool not affiliated with Roblox Corporation.
