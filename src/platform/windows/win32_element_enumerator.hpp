#pragma once

#include "../../core/element_enumerator.hpp"
#include "platform_config.h"

#ifdef WINDOWS_PLATFORM

#include <windows.h>
#include <uiautomation.h>
#include <map>

namespace WindowManager {

/**
 * Windows-specific element enumerator using UI Automation
 */
class Win32ElementEnumerator : public ElementEnumerator {
public:
    Win32ElementEnumerator();
    ~Win32ElementEnumerator() override;

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
    // COM and UI Automation interfaces
    IUIAutomation* uiAutomation_;
    bool comInitialized_;

    // Element cache for performance
    std::map<std::string, std::vector<UIElement>> elementCache_;
    std::map<std::string, std::chrono::steady_clock::time_point> cacheTimestamps_;

    // Helper methods for UI Automation
    bool initializeUIAutomation();
    void cleanup();
    IUIAutomationElement* getWindowElement(const std::string& windowHandle);
    UIElement createUIElementFromAutomationElement(IUIAutomationElement* element,
                                                   const std::string& parentWindowHandle);
    std::vector<UIElement> traverseElementTree(IUIAutomationElement* rootElement,
                                              const std::string& windowHandle);

    // Utility methods
    std::string handleToString(HWND hwnd);
    HWND stringToHandle(const std::string& handleStr);
    ElementType getElementTypeFromAutomation(IUIAutomationElement* element);
    ElementState getElementStateFromAutomation(IUIAutomationElement* element);
    bool isElementVisible(IUIAutomationElement* element);
    std::string getElementText(IUIAutomationElement* element);
    std::string getElementName(IUIAutomationElement* element);

    // Cache management
    bool isCacheValid(const std::string& windowHandle) const;
    void updateCache(const std::string& windowHandle, const std::vector<UIElement>& elements);
    std::vector<UIElement> getCachedElements(const std::string& windowHandle) const;

    // Error handling
    std::string getLastErrorMessage() const;

    // Performance tracking
    static constexpr std::chrono::milliseconds CACHE_TIMEOUT{30000}; // 30 seconds
};

} // namespace WindowManager

#endif // WINDOWS_PLATFORM