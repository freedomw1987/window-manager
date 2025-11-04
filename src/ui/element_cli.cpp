#include "element_cli.hpp"
#include "../core/element_types.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>

namespace WindowManager {

ElementCLI::ElementCLI()
    : outputFormat_("text")
    , verbose_(false) {
}

void ElementCLI::setOutputFormat(const std::string& format) {
    outputFormat_ = format;
}

void ElementCLI::setVerbose(bool verbose) {
    verbose_ = verbose;
}

void ElementCLI::displayElements(const ElementEnumerationResult& result) {
    if (outputFormat_ == "json") {
        std::cout << result.toJson() << std::endl;
        return;
    }

    // Text format display
    std::cout << "Elements in window " << result.windowHandle;
    if (!result.windowTitle.empty()) {
        std::cout << " (\"" << result.windowTitle << "\")";
    }
    std::cout << ":\n\n";

    if (result.elements.empty()) {
        std::cout << "No elements found.\n";
        return;
    }

    printElementTableHeader();
    printElementSeparator();

    for (const auto& element : result.elements) {
        printElementTableRow(element);
    }

    std::cout << "\nFound " << result.totalElementCount << " elements";
    if (result.filteredElementCount != result.totalElementCount) {
        std::cout << " (" << result.filteredElementCount << " after filtering)";
    }
    std::cout << " in " << result.enumerationTime.count() << " ms\n";

    if (verbose_) {
        displayElementPerformanceStats(result.enumerationTime, result.totalElementCount, result.windowHandle);
    }
}

void ElementCLI::displayElementsCompact(const ElementEnumerationResult& result, bool handlesOnly) {
    if (outputFormat_ == "json") {
        std::cout << result.toJson() << std::endl;
        return;
    }

    for (const auto& element : result.elements) {
        if (handlesOnly) {
            std::cout << element.handle << "\t" << elementTypeToString(element.type) << std::endl;
        } else {
            std::cout << element.handle << "\t" << elementTypeToString(element.type)
                      << "\t\"" << element.name << "\"" << std::endl;
        }
    }
}

void ElementCLI::displayElementSearchResults(const ElementEnumerationResult& result,
                                           const std::string& searchTerm) {
    if (outputFormat_ == "json") {
        std::cout << result.toJson() << std::endl;
        return;
    }

    std::cout << "Search results for \"" << searchTerm << "\" in window " << result.windowHandle;
    if (!result.windowTitle.empty()) {
        std::cout << " (\"" << result.windowTitle << "\")";
    }
    std::cout << ":\n\n";

    if (result.elements.empty()) {
        displayNoElementsFound(result.windowHandle, searchTerm);
        return;
    }

    printElementTableHeader();
    printElementSeparator();

    for (const auto& element : result.elements) {
        printElementTableRow(element);
    }

    std::cout << "\nFound " << result.filteredElementCount << " matches out of "
              << result.totalElementCount << " total elements";
    std::cout << " (search time: " << result.enumerationTime.count() << "ms)\n";
}

void ElementCLI::displayElementDetails(const UIElement& element, bool includeHierarchy) {
    if (outputFormat_ == "json") {
        std::cout << element.toJson() << std::endl;
        return;
    }

    std::cout << "Element Details:\n";
    std::cout << "  Handle: " << element.handle << "\n";
    std::cout << "  Type: " << elementTypeToString(element.type) << "\n";
    std::cout << "  Name: \"" << element.name << "\"\n";

    if (!element.value.empty()) {
        std::cout << "  Value: \"" << element.value << "\"\n";
    }

    std::cout << "  Position: (" << element.x << ", " << element.y << ")\n";
    std::cout << "  Size: " << element.width << "x" << element.height << "\n";
    std::cout << "  State: " << elementStateToString(element.state) << "\n";
    std::cout << "  Visible: " << (element.isVisible ? "Yes" : "No") << "\n";
    std::cout << "  Enabled: " << (element.isEnabled ? "Yes" : "No") << "\n";
    std::cout << "  Focusable: " << (element.isFocusable ? "Yes" : "No") << "\n";
    std::cout << "  Clickable: " << (element.isClickable ? "Yes" : "No") << "\n";

    if (!element.description.empty()) {
        std::cout << "  Description: \"" << element.description << "\"\n";
    }

    if (!element.accessibilityLabel.empty()) {
        std::cout << "  Accessibility Label: \"" << element.accessibilityLabel << "\"\n";
    }

    if (includeHierarchy) {
        std::cout << "  Parent Window: " << element.parentWindowHandle << "\n";
        if (!element.parentElementHandle.empty()) {
            std::cout << "  Parent Element: " << element.parentElementHandle << "\n";
        }
    }
}

void ElementCLI::displayNoElementsFound(const std::string& windowHandle, const std::string& searchTerm) {
    if (searchTerm.empty()) {
        std::cout << "No elements found in window " << windowHandle << ".\n";
        std::cout << "This could mean:\n";
        std::cout << "  - The window has no accessible UI elements\n";
        std::cout << "  - Element enumeration is not supported for this window\n";
        std::cout << "  - Accessibility permissions may be required\n";
    } else {
        std::cout << "No elements matching \"" << searchTerm << "\" found in window " << windowHandle << ".\n";
        std::cout << "Try:\n";
        std::cout << "  - Using a different search term\n";
        std::cout << "  - Searching without case sensitivity\n";
        std::cout << "  - Using partial matches instead of exact matches\n";
    }
}

void ElementCLI::displayElementEnumerationError(const std::string& windowHandle,
                                              const std::string& errorMessage,
                                              const std::string& suggestion) {
    std::cout << "Error enumerating elements in window " << windowHandle << ":\n";
    std::cout << "  " << errorMessage << "\n";
    if (!suggestion.empty()) {
        std::cout << "\nSuggestion: " << suggestion << "\n";
    }
}

void ElementCLI::displayPermissionWarning(const std::string& platform) {
    std::cout << "Warning: Element enumeration may require accessibility permissions.\n";
    std::cout << "Platform: " << platform << "\n";
    std::cout << "Please check system settings for accessibility permissions.\n";
}

void ElementCLI::displayElementPerformanceStats(std::chrono::milliseconds duration,
                                              size_t elementCount,
                                              const std::string& windowHandle) {
    std::cout << "\nPerformance Statistics:\n";
    std::cout << "  Enumeration time: " << duration.count() << "ms";
    if (duration.count() <= 2000) {
        std::cout << " ✓ (within 2 second target)";
    } else {
        std::cout << " ✗ (exceeds 2 second target)";
    }
    std::cout << "\n";
    std::cout << "  Elements discovered: " << elementCount << "\n";
    std::cout << "  Window: " << windowHandle << "\n";
}

void ElementCLI::displayInfo(const std::string& message) {
    std::cout << "Info: " << message << std::endl;
}

void ElementCLI::displaySuccess(const std::string& message) {
    std::cout << "Success: " << message << std::endl;
}

void ElementCLI::displayError(const std::string& message) {
    std::cerr << "Error: " << message << std::endl;
}

// Helper methods implementation
std::string ElementCLI::formatElement(const UIElement& element) const {
    if (outputFormat_ == "json") {
        return formatElementAsJson(element);
    } else {
        return formatElementAsText(element);
    }
}

std::string ElementCLI::formatElementAsJson(const UIElement& element) const {
    return element.toJson();
}

std::string ElementCLI::formatElementAsText(const UIElement& element) const {
    std::ostringstream oss;
    oss << std::left << std::setw(15) << element.handle.substr(0, 14);
    oss << std::setw(12) << elementTypeToString(element.type);
    oss << std::setw(25) << ("\"" + element.name.substr(0, 22) + "\"");
    oss << "(" << element.x << "," << element.y << ")";
    oss << std::setw(10) << elementStateToString(element.state);
    return oss.str();
}

void ElementCLI::printElementTableHeader() const {
    std::cout << std::left;
    std::cout << std::setw(15) << "Handle";
    std::cout << std::setw(12) << "Type";
    std::cout << std::setw(25) << "Name";
    std::cout << std::setw(12) << "Position";
    std::cout << std::setw(10) << "State";
    std::cout << "\n";
}

void ElementCLI::printElementTableRow(const UIElement& element) const {
    std::cout << std::left;
    std::cout << std::setw(15) << element.handle.substr(0, 14);
    std::cout << std::setw(12) << elementTypeToString(element.type);
    std::cout << std::setw(25) << ("\"" + element.name.substr(0, 22) + "\"");

    std::ostringstream pos;
    pos << "(" << element.x << "," << element.y << ")";
    std::cout << std::setw(12) << pos.str();

    std::cout << std::setw(10) << elementStateToString(element.state);
    std::cout << "\n";
}

void ElementCLI::printElementSeparator() const {
    std::cout << std::string(74, '-') << "\n";
}

std::string ElementCLI::highlightSearchTerm(const std::string& text, const std::string& searchTerm) const {
    // Simple implementation - could be enhanced with color codes
    return text; // For now, return as-is
}

std::string ElementCLI::elementTypeToDisplayString(ElementType type) const {
    return elementTypeToString(type);
}

std::string ElementCLI::elementStateToDisplayString(ElementState state) const {
    return elementStateToString(state);
}

std::string ElementCLI::formatElementsAsJsonArray(const std::vector<UIElement>& elements) const {
    std::ostringstream oss;
    oss << "[";
    for (size_t i = 0; i < elements.size(); ++i) {
        if (i > 0) oss << ",";
        oss << elements[i].toJson();
    }
    oss << "]";
    return oss.str();
}

std::string ElementCLI::escapeJsonString(const std::string& str) const {
    std::string result;
    for (char c : str) {
        switch (c) {
            case '"': result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            default: result += c; break;
        }
    }
    return result;
}

} // namespace WindowManager