#pragma once

#include "../../../src/core/window.hpp"
#include <vector>
#include <string>

namespace WindowManager {

// Forward declarations
struct UIElement;
struct ElementEnumerationResult;

/**
 * Command Line Interface extensions for element operations
 * Extends existing CLI class to support element display and formatting
 */
class ElementCLI {
public:
    ElementCLI();
    ~ElementCLI() = default;

    // Configuration
    void setOutputFormat(const std::string& format);  // "text" or "json"
    void setVerbose(bool verbose);

    // Element display methods

    /**
     * Display all elements discovered within a window
     * @param result ElementEnumerationResult from enumeration
     */
    void displayElements(const ElementEnumerationResult& result);

    /**
     * Display elements in compact format (handles and names only)
     * @param result ElementEnumerationResult from enumeration
     * @param handlesOnly If true, show only handles and types
     */
    void displayElementsCompact(const ElementEnumerationResult& result,
                               bool handlesOnly = false);

    /**
     * Display element search results with highlighting
     * @param result ElementEnumerationResult from search operation
     * @param searchTerm The search term used for highlighting
     */
    void displayElementSearchResults(const ElementEnumerationResult& result,
                                   const std::string& searchTerm);

    /**
     * Display detailed information about a single element
     * @param element UIElement to display
     * @param includeHierarchy If true, show parent/child relationships
     */
    void displayElementDetails(const UIElement& element,
                              bool includeHierarchy = false);

    // Status and error messages

    /**
     * Display message when no elements are found
     * @param windowHandle The window that was searched
     * @param searchTerm Optional search term that produced no results
     */
    void displayNoElementsFound(const std::string& windowHandle,
                               const std::string& searchTerm = "");

    /**
     * Display error when element enumeration fails
     * @param windowHandle The window that failed enumeration
     * @param errorMessage Specific error description
     * @param suggestion Helpful suggestion for resolving the error
     */
    void displayElementEnumerationError(const std::string& windowHandle,
                                      const std::string& errorMessage,
                                      const std::string& suggestion);

    /**
     * Display warning when element access permissions are missing
     * @param platform Platform-specific permission guidance
     */
    void displayPermissionWarning(const std::string& platform);

    /**
     * Display element enumeration performance statistics
     * @param duration Time taken for enumeration
     * @param elementCount Number of elements discovered
     * @param windowHandle Window that was enumerated
     */
    void displayElementPerformanceStats(std::chrono::milliseconds duration,
                                       size_t elementCount,
                                       const std::string& windowHandle);

    // Utility methods

    /**
     * Display general information message
     * @param message Information to display
     */
    void displayInfo(const std::string& message);

    /**
     * Display success message
     * @param message Success information to display
     */
    void displaySuccess(const std::string& message);

    /**
     * Display error message
     * @param message Error information to display
     */
    void displayError(const std::string& message);

private:
    std::string outputFormat_;  // "text" or "json"
    bool verbose_;

    // Helper methods for formatting
    std::string formatElement(const UIElement& element) const;
    std::string formatElementAsJson(const UIElement& element) const;
    std::string formatElementAsText(const UIElement& element) const;
    std::string highlightSearchTerm(const std::string& text,
                                   const std::string& searchTerm) const;
    std::string elementTypeToString(ElementType type) const;
    std::string elementStateToString(ElementState state) const;
};

} // namespace WindowManager