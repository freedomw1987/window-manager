#pragma once
#include <string>

namespace WindowManager {

/**
 * Enumeration of UI element types
 * Represents different kinds of UI controls and components
 */
enum class ElementType {
    Unknown,
    Button,
    TextField,
    Label,
    Menu,
    MenuItem,
    ComboBox,
    CheckBox,
    RadioButton,
    Image,
    Link,
    Table,
    Cell,
    Window,
    Pane,
    ScrollBar,
    Slider,
    ProgressBar
};

/**
 * Enumeration of UI element states
 * Represents the current state of a UI element
 */
enum class ElementState {
    Normal,
    Disabled,
    Hidden,
    Focused,
    Selected,
    Checked,
    Expanded,
    Collapsed
};

/**
 * Utility functions for ElementType
 */
const char* elementTypeToString(ElementType type);
ElementType stringToElementType(const std::string& str);

/**
 * Utility functions for ElementState
 */
const char* elementStateToString(ElementState state);
ElementState stringToElementState(const std::string& str);

} // namespace WindowManager