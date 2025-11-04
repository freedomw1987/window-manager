#include "ui_element.hpp"
#include <sstream>
#include <iomanip>

namespace WindowManager {

// Default constructor
UIElement::UIElement()
    : type(ElementType::Unknown)
    , x(0), y(0), width(0), height(0)
    , isVisible(true)
    , state(ElementState::Normal)
    , isEnabled(true)
    , isFocusable(false)
    , isClickable(false)
    , discoveredAt(std::chrono::steady_clock::now())
{
}

// Parameterized constructor
UIElement::UIElement(const std::string& handle, const std::string& parentWindow,
                     ElementType type, const std::string& name)
    : handle(handle)
    , parentWindowHandle(parentWindow)
    , type(type)
    , name(name)
    , x(0), y(0), width(0), height(0)
    , isVisible(true)
    , state(ElementState::Normal)
    , isEnabled(true)
    , isFocusable(false)
    , isClickable(false)
    , discoveredAt(std::chrono::steady_clock::now())
{
}

// Validation methods
bool UIElement::isValid() const {
    return !handle.empty() && !parentWindowHandle.empty();
}

bool UIElement::hasValidPosition() const {
    // Position can be negative (elements outside visible area)
    // But we need valid dimensions for visible elements
    return isVisible ? hasValidDimensions() : true;
}

bool UIElement::hasValidDimensions() const {
    return width > 0 && height > 0;
}

// Output formatting methods
std::string UIElement::toString() const {
    std::ostringstream oss;
    oss << "Element[" << handle << "] "
        << "Type: " << elementTypeToString(type) << ", "
        << "Name: \"" << name << "\", "
        << "Position: (" << x << "," << y << "), "
        << "Size: " << width << "x" << height << ", "
        << "State: " << elementStateToString(state) << ", "
        << "Visible: " << (isVisible ? "yes" : "no") << ", "
        << "Enabled: " << (isEnabled ? "yes" : "no");

    if (!value.empty()) {
        oss << ", Value: \"" << value << "\"";
    }

    return oss.str();
}

std::string UIElement::toJson() const {
    std::ostringstream oss;
    oss << "{";
    oss << "\"handle\":\"" << handle << "\",";
    oss << "\"parentWindow\":\"" << parentWindowHandle << "\",";
    oss << "\"type\":\"" << elementTypeToString(type) << "\",";
    oss << "\"name\":\"" << name << "\",";
    oss << "\"position\":{\"x\":" << x << ",\"y\":" << y << "},";
    oss << "\"size\":{\"width\":" << width << ",\"height\":" << height << "},";
    oss << "\"state\":\"" << elementStateToString(state) << "\",";
    oss << "\"visible\":" << (isVisible ? "true" : "false") << ",";
    oss << "\"enabled\":" << (isEnabled ? "true" : "false") << ",";
    oss << "\"focusable\":" << (isFocusable ? "true" : "false") << ",";
    oss << "\"clickable\":" << (isClickable ? "true" : "false");

    if (!value.empty()) {
        oss << ",\"value\":\"" << value << "\"";
    }
    if (!description.empty()) {
        oss << ",\"description\":\"" << description << "\"";
    }
    if (!accessibilityLabel.empty()) {
        oss << ",\"accessibilityLabel\":\"" << accessibilityLabel << "\"";
    }

    oss << "}";
    return oss.str();
}

std::string UIElement::toCompactString() const {
    std::ostringstream oss;
    oss << elementTypeToString(type) << " \"" << name << "\"";
    if (!value.empty() && value != name) {
        oss << " = \"" << value << "\"";
    }
    oss << " (" << x << "," << y << ")";
    return oss.str();
}

// Comparison operators
bool UIElement::operator==(const UIElement& other) const {
    return handle == other.handle &&
           parentWindowHandle == other.parentWindowHandle;
}

bool UIElement::operator!=(const UIElement& other) const {
    return !(*this == other);
}

bool UIElement::operator<(const UIElement& other) const {
    // Sort by window first, then by position (top to bottom, left to right)
    if (parentWindowHandle != other.parentWindowHandle) {
        return parentWindowHandle < other.parentWindowHandle;
    }
    if (y != other.y) {
        return y < other.y;
    }
    return x < other.x;
}

} // namespace WindowManager