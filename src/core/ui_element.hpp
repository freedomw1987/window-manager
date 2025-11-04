#pragma once

#include <string>
#include <chrono>
#include <vector>
#include <map>
#include "element_types.hpp"

namespace WindowManager {

/**
 * Core UI Element structure
 * Represents a single UI element within an application window
 */
struct UIElement {
    // Identity
    std::string handle;                 // Platform-specific element identifier
    std::string parentWindowHandle;     // Reference to containing window
    std::string parentElementHandle;    // Reference to parent element (for hierarchy)

    // Basic Properties
    ElementType type;
    std::string name;                   // Element name/title
    std::string value;                  // Element value/text content
    std::string description;            // Accessibility description
    std::string role;                   // Accessibility role

    // Position and Size (relative to window)
    int x, y;                          // Position relative to parent window
    unsigned int width, height;       // Element dimensions
    bool isVisible;                    // Element visibility state

    // State Information
    ElementState state;
    bool isEnabled;
    bool isFocusable;
    bool isClickable;

    // Accessibility Properties
    std::string accessibilityLabel;
    std::string accessibilityHelp;
    std::string accessibilityValue;

    // Platform-specific metadata (JSON string for flexibility)
    std::string platformData;

    // Timestamps
    std::chrono::steady_clock::time_point discoveredAt;

    // Constructors
    UIElement();
    UIElement(const std::string& handle, const std::string& parentWindow,
              ElementType type, const std::string& name);

    // Validation methods
    bool isValid() const;
    bool hasValidPosition() const;
    bool hasValidDimensions() const;

    // Output formatting methods
    std::string toString() const;
    std::string toJson() const;
    std::string toCompactString() const;

    // Comparison operators
    bool operator==(const UIElement& other) const;
    bool operator!=(const UIElement& other) const;
    bool operator<(const UIElement& other) const;
};

} // namespace WindowManager