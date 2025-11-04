#pragma once

#include "../../core/element_enumerator.hpp"
#include "platform_config.h"

#ifdef MACOS_PLATFORM

#include <CoreGraphics/CoreGraphics.h>
#include <CoreFoundation/CoreFoundation.h>
#include <ApplicationServices/ApplicationServices.h>
#include <map>

namespace WindowManager {

/**
 * macOS-specific element enumerator using Accessibility APIs
 */
class CocoaElementEnumerator : public ElementEnumerator {
public:
    CocoaElementEnumerator();
    ~CocoaElementEnumerator() override = default;

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
    // Element cache for performance
    std::map<std::string, std::vector<UIElement>> elementCache_;
    std::map<std::string, std::chrono::steady_clock::time_point> cacheTimestamps_;

    // Helper methods for Accessibility API
    AXUIElementRef getWindowElement(const std::string& windowHandle);
    UIElement createUIElementFromAXElement(AXUIElementRef element,
                                          const std::string& parentWindowHandle);
    std::vector<UIElement> traverseElementTree(AXUIElementRef rootElement,
                                              const std::string& windowHandle);

    // Utility methods
    std::string handleToString(CGWindowID windowId);
    CGWindowID stringToHandle(const std::string& handleStr);
    ElementType getElementTypeFromAX(AXUIElementRef element);
    ElementState getElementStateFromAX(AXUIElementRef element);
    bool isElementVisible(AXUIElementRef element);
    std::string getElementText(AXUIElementRef element);
    std::string getElementName(AXUIElementRef element);
    std::string getElementRole(AXUIElementRef element);

    // Accessibility API helpers
    CFStringRef getStringAttribute(AXUIElementRef element, CFStringRef attribute);
    CFTypeRef getAttribute(AXUIElementRef element, CFStringRef attribute);
    bool getBoolAttribute(AXUIElementRef element, CFStringRef attribute);
    CGRect getElementBounds(AXUIElementRef element);
    std::vector<AXUIElementRef> getElementChildren(AXUIElementRef element);

    // Cache management
    bool isCacheValid(const std::string& windowHandle) const;
    void updateCache(const std::string& windowHandle, const std::vector<UIElement>& elements);
    std::vector<UIElement> getCachedElements(const std::string& windowHandle) const;

    // Application-specific element creation (legacy - deprecated)
    void createWordElements(std::vector<UIElement>& elements, const std::string& windowHandle, const std::string& windowTitle);
    void createBrowserElements(std::vector<UIElement>& elements, const std::string& windowHandle, const std::string& windowTitle);
    void createTerminalElements(std::vector<UIElement>& elements, const std::string& windowHandle, const std::string& windowTitle);
    void createGenericElements(std::vector<UIElement>& elements, const std::string& windowHandle, const std::string& windowTitle);

    // Real Accessibility API methods
    void traverseElementTreeRecursive(AXUIElementRef element, const std::string& windowHandle, std::vector<UIElement>& elements, int depth, int maxDepth);
    ElementType getElementTypeFromRole(const std::string& role);

    // Error handling and permissions
    bool checkAccessibilityPermissions() const;
    std::string getAccessibilityErrorMessage() const;

    // Performance tracking
    static constexpr std::chrono::milliseconds CACHE_TIMEOUT{30000}; // 30 seconds
};

} // namespace WindowManager

#endif // MACOS_PLATFORM