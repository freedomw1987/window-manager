#pragma once

#include <vector>
#include <string>
#include <chrono>
#include "ui_element.hpp"

namespace WindowManager {

/**
 * Container for element enumeration results with metadata
 * Holds the results of element discovery operations along with timing and status information
 */
struct ElementEnumerationResult {
    // Results
    std::vector<UIElement> elements;
    std::string windowHandle;
    std::string windowTitle;

    // Metadata
    size_t totalElementCount;
    size_t filteredElementCount;
    std::chrono::milliseconds enumerationTime;
    std::chrono::steady_clock::time_point enumeratedAt;

    // Status information
    bool success;
    std::string errorMessage;
    bool hasAccessibilityPermissions;
    bool supportsElementEnumeration;

    // Constructor
    ElementEnumerationResult();
    ElementEnumerationResult(const std::string& windowHandle);

    // Performance metrics
    bool meetsPerformanceTarget() const;

    // Validation methods
    bool isValid() const;
    bool isEmpty() const;
    bool hasErrors() const;

    // Output formatting methods
    std::string getSummary() const;
    std::string toJson() const;
    std::string getPerformanceInfo() const;

    // Element access helpers
    void addElement(const UIElement& element);
    void clearElements();
    size_t getElementCount() const;
};

} // namespace WindowManager