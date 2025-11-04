#include "element_enumerator.hpp"
#include "platform_config.h"

// Platform-specific includes
#ifdef WINDOWS_PLATFORM
#include "../platform/windows/win32_element_enumerator.hpp"
#elif defined(MACOS_PLATFORM)
#include "../platform/macos/cocoa_element_enumerator.hpp"
#elif defined(LINUX_PLATFORM)
#include "../platform/linux/x11_element_enumerator.hpp"
#endif

namespace WindowManager {

// Factory method implementation
std::unique_ptr<ElementEnumerator> ElementEnumerator::create() {
#ifdef WINDOWS_PLATFORM
    return std::make_unique<Win32ElementEnumerator>();
#elif defined(MACOS_PLATFORM)
    return std::make_unique<CocoaElementEnumerator>();
#elif defined(LINUX_PLATFORM)
    return std::make_unique<X11ElementEnumerator>();
#else
    // Return null for unsupported platforms
    return nullptr;
#endif
}

// Helper method for timing
void ElementEnumerator::updateEnumerationTime(const std::chrono::steady_clock::time_point& start,
                                            const std::chrono::steady_clock::time_point& end) {
    lastEnumerationTime_ = start;
    lastEnumerationDuration_ = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
}

} // namespace WindowManager