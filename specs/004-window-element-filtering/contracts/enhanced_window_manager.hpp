#pragma once

#include "../../../src/core/window_manager.hpp"
#include "element_enumerator.hpp"
#include <memory>

namespace WindowManager {

// Forward declarations
struct ElementEnumerationResult;
struct ElementSearchQuery;

/**
 * Enhanced Window Manager with element enumeration capabilities
 * Extends existing WindowManager to support UI element operations
 */
class EnhancedWindowManager : public WindowManager {
public:
    EnhancedWindowManager();
    ~EnhancedWindowManager() override = default;

    // Element enumeration operations

    /**
     * List all UI elements within a specific window
     * @param windowHandle Platform-specific window identifier
     * @return ElementEnumerationResult containing all discovered elements
     */
    ElementEnumerationResult listWindowElements(const std::string& windowHandle);

    /**
     * Search for specific elements within a window
     * @param windowHandle Platform-specific window identifier
     * @param query Search criteria and filters
     * @return ElementEnumerationResult containing matching elements
     */
    ElementEnumerationResult searchWindowElements(const std::string& windowHandle,
                                                 const ElementSearchQuery& query);

    /**
     * Get detailed information about a specific element
     * @param elementHandle Platform-specific element identifier
     * @return UIElement information if found, std::nullopt otherwise
     */
    std::optional<UIElement> getElementDetails(const std::string& elementHandle);

    // Window capability checks

    /**
     * Check if a window supports element enumeration
     * @param windowHandle Platform-specific window identifier
     * @return true if window supports element discovery
     */
    bool windowSupportsElements(const std::string& windowHandle);

    /**
     * Check if platform has required permissions for element access
     * @return true if accessibility permissions are available
     */
    bool hasElementAccessPermissions() const;

    /**
     * Get estimated number of elements in a window (for performance planning)
     * @param windowHandle Platform-specific window identifier
     * @return Estimated element count, 0 if unknown
     */
    size_t getEstimatedElementCount(const std::string& windowHandle);

    // Performance and optimization

    /**
     * Warm up element cache for a specific window
     * @param windowHandle Platform-specific window identifier
     * @return true if cache was successfully warmed
     */
    bool warmElementCache(const std::string& windowHandle);

    /**
     * Clear element cache for better memory management
     * @param windowHandle Optional specific window, empty for all windows
     */
    void clearElementCache(const std::string& windowHandle = "");

    /**
     * Check if element enumeration meets performance requirements
     * @param windowHandle Window to check
     * @return true if performance targets are met
     */
    bool meetsElementPerformanceRequirements(const std::string& windowHandle);

    // Platform information

    /**
     * Get element enumeration capabilities for current platform
     * @return String describing supported features and limitations
     */
    std::string getElementEnumerationCapabilities() const;

    /**
     * Get platform-specific guidance for element access setup
     * @return Setup instructions for current platform
     */
    std::string getElementAccessSetupGuidance() const;

    // Factory method
    static std::unique_ptr<EnhancedWindowManager> create();

private:
    std::unique_ptr<ElementEnumerator> elementEnumerator_;

    // Helper methods
    bool validateWindowHandle(const std::string& windowHandle) const;
    void logElementOperation(const std::string& operation,
                           const std::string& windowHandle,
                           std::chrono::milliseconds duration) const;
};

} // namespace WindowManager