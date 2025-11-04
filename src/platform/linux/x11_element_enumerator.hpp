#pragma once

#include "../../core/element_enumerator.hpp"
#include "platform_config.h"

#ifdef LINUX_PLATFORM

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <map>
#include <string>

namespace WindowManager {

/**
 * Linux-specific element enumerator using X11 and AT-SPI2
 * Note: Limited functionality compared to Windows/macOS due to X11 architecture
 */
class X11ElementEnumerator : public ElementEnumerator {
public:
    X11ElementEnumerator();
    ~X11ElementEnumerator() override;

    // ElementEnumerator interface implementation
    ElementEnumerationResult enumerateElements(const std::string& windowHandle) override;
    ElementEnumerationResult searchElements(const std::string& windowHandle,
                                          const ElementSearchQuery& query) override;
    std::optional<UIElement> getElementInfo(const std::string& elementHandle) override;
    bool isElementValid(const std::string& elementHandle) override;

    // Window capability checks
    bool supportsElementEnumeration(const std::string& windowHandle) override;
    bool hasElementAccessPermissions() const override;

    // Performance and caching
    void clearElementCache(const std::string& windowHandle) override;
    void clearAllElementCaches() override;
    std::chrono::milliseconds getLastEnumerationTime() const override;
    std::string getPlatformInfo() const override;

private:
    // X11 connection
    Display* display_;
    bool x11Connected_;

    // Element cache for performance
    std::map<std::string, std::vector<UIElement>> elementCache_;
    std::map<std::string, std::chrono::steady_clock::time_point> cacheTimestamps_;

    // Helper methods for X11 and AT-SPI
    bool initializeX11();
    void cleanup();
    Window getX11Window(const std::string& windowHandle);
    UIElement createUIElementFromX11(Window window, const std::string& parentWindowHandle);
    std::vector<UIElement> enumerateBasicElements(Window window, const std::string& windowHandle);

    // Basic X11 element discovery (limited functionality)
    std::vector<UIElement> findChildWindows(Window window, const std::string& windowHandle);
    std::vector<UIElement> analyzeWindowProperties(Window window, const std::string& windowHandle);

    // Utility methods
    std::string handleToString(Window window);
    Window stringToHandle(const std::string& handleStr);
    ElementType guessElementTypeFromX11(Window window);
    bool isWindowVisible(Window window);
    std::string getWindowText(Window window);
    std::string getWindowName(Window window);
    std::string getWindowClass(Window window);

    // X11 property helpers
    std::string getWindowProperty(Window window, Atom property);
    bool hasProperty(Window window, Atom property);
    std::vector<Window> getChildWindows(Window window);

    // AT-SPI integration (if available)
    bool tryATSPIEnumeration(const std::string& windowHandle, std::vector<UIElement>& elements);
    bool isATSPIAvailable() const;

    // Cache management
    bool isCacheValid(const std::string& windowHandle) const;
    void updateCache(const std::string& windowHandle, const std::vector<UIElement>& elements);
    std::vector<UIElement> getCachedElements(const std::string& windowHandle) const;

    // Error handling
    std::string getX11ErrorMessage() const;

    // Performance tracking
    static constexpr std::chrono::milliseconds CACHE_TIMEOUT{30000}; // 30 seconds

    // X11 atoms for common properties
    Atom atomWMName_;
    Atom atomWMClass_;
    Atom atomNetWMName_;
    Atom atomNetWMWindowType_;
};

} // namespace WindowManager

#endif // LINUX_PLATFORM