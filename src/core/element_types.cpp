#include "element_types.hpp"
#include <string>

namespace WindowManager {

// ElementType utility functions
const char* elementTypeToString(ElementType type) {
    switch (type) {
        case ElementType::Unknown: return "Unknown";
        case ElementType::Button: return "Button";
        case ElementType::TextField: return "TextField";
        case ElementType::Label: return "Label";
        case ElementType::Menu: return "Menu";
        case ElementType::MenuItem: return "MenuItem";
        case ElementType::ComboBox: return "ComboBox";
        case ElementType::CheckBox: return "CheckBox";
        case ElementType::RadioButton: return "RadioButton";
        case ElementType::Image: return "Image";
        case ElementType::Link: return "Link";
        case ElementType::Table: return "Table";
        case ElementType::Cell: return "Cell";
        case ElementType::Window: return "Window";
        case ElementType::Pane: return "Pane";
        case ElementType::ScrollBar: return "ScrollBar";
        case ElementType::Slider: return "Slider";
        case ElementType::ProgressBar: return "ProgressBar";
        default: return "Unknown";
    }
}

ElementType stringToElementType(const std::string& str) {
    if (str == "Button") return ElementType::Button;
    if (str == "TextField") return ElementType::TextField;
    if (str == "Label") return ElementType::Label;
    if (str == "Menu") return ElementType::Menu;
    if (str == "MenuItem") return ElementType::MenuItem;
    if (str == "ComboBox") return ElementType::ComboBox;
    if (str == "CheckBox") return ElementType::CheckBox;
    if (str == "RadioButton") return ElementType::RadioButton;
    if (str == "Image") return ElementType::Image;
    if (str == "Link") return ElementType::Link;
    if (str == "Table") return ElementType::Table;
    if (str == "Cell") return ElementType::Cell;
    if (str == "Window") return ElementType::Window;
    if (str == "Pane") return ElementType::Pane;
    if (str == "ScrollBar") return ElementType::ScrollBar;
    if (str == "Slider") return ElementType::Slider;
    if (str == "ProgressBar") return ElementType::ProgressBar;
    return ElementType::Unknown;
}

// ElementState utility functions
const char* elementStateToString(ElementState state) {
    switch (state) {
        case ElementState::Normal: return "Normal";
        case ElementState::Disabled: return "Disabled";
        case ElementState::Hidden: return "Hidden";
        case ElementState::Focused: return "Focused";
        case ElementState::Selected: return "Selected";
        case ElementState::Checked: return "Checked";
        case ElementState::Expanded: return "Expanded";
        case ElementState::Collapsed: return "Collapsed";
        default: return "Normal";
    }
}

ElementState stringToElementState(const std::string& str) {
    if (str == "Normal") return ElementState::Normal;
    if (str == "Disabled") return ElementState::Disabled;
    if (str == "Hidden") return ElementState::Hidden;
    if (str == "Focused") return ElementState::Focused;
    if (str == "Selected") return ElementState::Selected;
    if (str == "Checked") return ElementState::Checked;
    if (str == "Expanded") return ElementState::Expanded;
    if (str == "Collapsed") return ElementState::Collapsed;
    return ElementState::Normal;
}

} // namespace WindowManager