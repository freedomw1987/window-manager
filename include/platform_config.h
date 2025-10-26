#pragma once

#include <chrono>

// Platform detection macros
// These are set by CMake based on the target platform

#if defined(WINDOWS_PLATFORM) || defined(_WIN32) || defined(_WIN64)
    #define WM_PLATFORM_WINDOWS
    #define WM_PLATFORM_NAME "Windows"
#elif defined(MACOS_PLATFORM) || defined(__APPLE__) && defined(__MACH__)
    #define WM_PLATFORM_MACOS
    #define WM_PLATFORM_NAME "macOS"
#elif defined(LINUX_PLATFORM) || defined(__linux__)
    #define WM_PLATFORM_LINUX
    #define WM_PLATFORM_NAME "Linux"
#else
    #define WM_PLATFORM_UNKNOWN
    #define WM_PLATFORM_NAME "Unknown"
    #error "Unsupported platform. This application supports Windows, macOS, and Linux only."
#endif

// Platform-specific includes and forward declarations
#ifdef WM_PLATFORM_WINDOWS
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>
    #include <psapi.h>
    #include <ShObjIdl.h>
    #include <comdef.h>
    namespace WindowManager { class Win32Enumerator; }
#endif

#ifdef WM_PLATFORM_MACOS
    // Forward declarations for macOS - actual includes in implementation files
    namespace WindowManager { class CocoaEnumerator; }
#endif

#ifdef WM_PLATFORM_LINUX
    #include <X11/Xlib.h>
    #include <X11/Xutil.h>
    #include <X11/Xatom.h>
    namespace WindowManager { class X11Enumerator; }
#endif

// Compiler-specific attributes
#ifdef _MSC_VER
    #define WM_FORCE_INLINE __forceinline
    #define WM_NO_INLINE __declspec(noinline)
#elif defined(__GNUC__) || defined(__clang__)
    #define WM_FORCE_INLINE __attribute__((always_inline)) inline
    #define WM_NO_INLINE __attribute__((noinline))
#else
    #define WM_FORCE_INLINE inline
    #define WM_NO_INLINE
#endif

// Platform-specific constants
#ifdef WM_PLATFORM_WINDOWS
    constexpr int WM_MAX_WINDOW_TITLE_LENGTH = 256;
    constexpr int WM_MAX_PROCESS_NAME_LENGTH = 260;
#elif defined(WM_PLATFORM_MACOS)
    constexpr int WM_MAX_WINDOW_TITLE_LENGTH = 512;
    constexpr int WM_MAX_PROCESS_NAME_LENGTH = 256;
#elif defined(WM_PLATFORM_LINUX)
    constexpr int WM_MAX_WINDOW_TITLE_LENGTH = 512;
    constexpr int WM_MAX_PROCESS_NAME_LENGTH = 256;
#endif

// Performance optimization hints
namespace WindowManager {

    // Platform-specific optimization settings
    struct PlatformConfig {
        static constexpr int DEFAULT_CACHE_SIZE = 100;
        static constexpr int MAX_WINDOWS_SUPPORTED = 1000;
        static constexpr std::chrono::milliseconds DEFAULT_REFRESH_INTERVAL{1000};

#ifdef WM_PLATFORM_WINDOWS
        static constexpr bool SUPPORTS_WINDOW_FOCUS = true;
        static constexpr bool REQUIRES_ADMIN_PRIVILEGES = false;
        static constexpr bool SUPPORTS_WORKSPACES = true;
        static constexpr bool SUPPORTS_VIRTUAL_DESKTOPS = true;
        static constexpr bool REQUIRES_COM_INITIALIZATION = true;
#elif defined(WM_PLATFORM_MACOS)
        static constexpr bool SUPPORTS_WINDOW_FOCUS = true;
        static constexpr bool REQUIRES_ADMIN_PRIVILEGES = false; // But needs accessibility permissions
        static constexpr bool SUPPORTS_WORKSPACES = true;
        static constexpr bool SUPPORTS_VIRTUAL_DESKTOPS = false; // Limited - uses Spaces
        static constexpr bool REQUIRES_ACCESSIBILITY_PERMISSIONS = true;
#elif defined(WM_PLATFORM_LINUX)
        static constexpr bool SUPPORTS_WINDOW_FOCUS = true;
        static constexpr bool REQUIRES_ADMIN_PRIVILEGES = false;
        static constexpr bool SUPPORTS_WORKSPACES = true;
        static constexpr bool SUPPORTS_VIRTUAL_DESKTOPS = true;
        static constexpr bool SUPPORTS_EWMH = true;
#endif
    };

} // namespace WindowManager