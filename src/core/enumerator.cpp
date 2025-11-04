#include "enumerator.hpp"
#include "exceptions.hpp"
#include "platform_config.h"

// Platform-specific includes and forward declarations
#ifdef WM_PLATFORM_WINDOWS
    #include "../platform/windows/win32_enumerator.hpp"
#elif defined(WM_PLATFORM_MACOS)
    #include "../platform/macos/cocoa_enumerator.hpp"
#elif defined(WM_PLATFORM_LINUX)
    #include "../platform/linux/x11_enumerator.hpp"
#endif

namespace WindowManager {

// Factory method implementation
std::unique_ptr<WindowEnumerator> WindowEnumerator::create() {
#ifdef WM_PLATFORM_WINDOWS
    return std::make_unique<Win32Enumerator>();
#elif defined(WM_PLATFORM_MACOS)
    return std::make_unique<CocoaEnumerator>();
#elif defined(WM_PLATFORM_LINUX)
    return std::make_unique<X11Enumerator>();
#else
    throw PlatformNotSupportedException(WM_PLATFORM_NAME);
#endif
}

// Helper method for updating timing information
void WindowEnumerator::updateEnumerationTime(const std::chrono::steady_clock::time_point& start,
                                            const std::chrono::steady_clock::time_point& end) {
    lastEnumerationTime_ = start;
    lastEnumerationDuration_ = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
}

} // namespace WindowManager