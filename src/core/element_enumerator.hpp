#pragma once

#include "ui_element.hpp"
#include "element_result.hpp"
#include "element_query.hpp"
#include <vector>
#include <memory>
#include <chrono>
#include <optional>

namespace WindowManager {

/**
 * UI Element enumeration interface
 * Abstract base class for platform-specific UI element discovery within windows
 */
class ElementEnumerator {
public:
    virtual ~ElementEnumerator() = default;

    // Core element enumeration operations

    /**
     * Enumerate all UI elements within a specific window
     * @param windowHandle Platform-specific window identifier
     * @return ElementEnumerationResult containing discovered elements and metadata
     */
    virtual ElementEnumerationResult enumerateElements(const std::string& windowHandle) = 0;

    /**
     * Search for specific elements within a window
     * @param windowHandle Platform-specific window identifier
     * @param query Search criteria and filters
     * @return ElementEnumerationResult containing matching elements
     */
    virtual ElementEnumerationResult searchElements(const std::string& windowHandle,
                                                   const ElementSearchQuery& query) = 0;

    /**
     * Get detailed information about a specific element
     * @param elementHandle Platform-specific element identifier
     * @return UIElement information if found, std::nullopt otherwise
     */
    virtual std::optional<UIElement> getElementInfo(const std::string& elementHandle) = 0;

    /**
     * Validate that an element handle is still valid and accessible
     * @param elementHandle Platform-specific element identifier
     * @return true if element exists and is accessible
     */
    virtual bool isElementValid(const std::string& elementHandle) = 0;

    // Window capability checks

    /**
     * Check if element enumeration is supported for a specific window
     * @param windowHandle Platform-specific window identifier
     * @return true if the window supports element enumeration
     */
    virtual bool supportsElementEnumeration(const std::string& windowHandle) = 0;

    /**
     * Check if platform has required permissions for element access
     * @return true if accessibility permissions are granted
     */
    virtual bool hasElementAccessPermissions() const = 0;

    // Performance and caching

    /**
     * Clear cached element data for a specific window
     * @param windowHandle Platform-specific window identifier
     */
    virtual void clearElementCache(const std::string& windowHandle) = 0;

    /**
     * Clear all cached element data
     */
    virtual void clearAllElementCaches() = 0;

    /**
     * Get the last enumeration time for performance monitoring
     * @return Duration of last element enumeration operation
     */
    virtual std::chrono::milliseconds getLastEnumerationTime() const = 0;

    /**
     * Get platform-specific information and capabilities
     * @return String describing platform and version information
     */
    virtual std::string getPlatformInfo() const = 0;

    // Factory method - creates platform-specific implementation
    static std::unique_ptr<ElementEnumerator> create();

protected:
    // Protected members for derived classes
    std::chrono::steady_clock::time_point lastEnumerationTime_;
    std::chrono::milliseconds lastEnumerationDuration_{0};

    // Helper method for updating timing information
    void updateEnumerationTime(const std::chrono::steady_clock::time_point& start,
                              const std::chrono::steady_clock::time_point& end);
};

} // namespace WindowManager