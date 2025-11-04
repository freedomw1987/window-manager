#include "element_result.hpp"
#include <sstream>
#include <algorithm>

namespace WindowManager {

// Default constructor
ElementEnumerationResult::ElementEnumerationResult()
    : totalElementCount(0)
    , filteredElementCount(0)
    , enumerationTime(std::chrono::milliseconds(0))
    , enumeratedAt(std::chrono::steady_clock::now())
    , success(false)
    , hasAccessibilityPermissions(false)
    , supportsElementEnumeration(false)
{
}

// Parameterized constructor
ElementEnumerationResult::ElementEnumerationResult(const std::string& windowHandle)
    : windowHandle(windowHandle)
    , totalElementCount(0)
    , filteredElementCount(0)
    , enumerationTime(std::chrono::milliseconds(0))
    , enumeratedAt(std::chrono::steady_clock::now())
    , success(false)
    , hasAccessibilityPermissions(false)
    , supportsElementEnumeration(false)
{
}

// Performance metrics
bool ElementEnumerationResult::meetsPerformanceTarget() const {
    return enumerationTime <= std::chrono::milliseconds(2000);
}

// Validation methods
bool ElementEnumerationResult::isValid() const {
    return success && !windowHandle.empty();
}

bool ElementEnumerationResult::isEmpty() const {
    return elements.empty();
}

bool ElementEnumerationResult::hasErrors() const {
    return !success || !errorMessage.empty();
}

// Element access helpers
void ElementEnumerationResult::addElement(const UIElement& element) {
    elements.push_back(element);
    totalElementCount = elements.size();
    filteredElementCount = elements.size();
}

void ElementEnumerationResult::clearElements() {
    elements.clear();
    totalElementCount = 0;
    filteredElementCount = 0;
}

size_t ElementEnumerationResult::getElementCount() const {
    return elements.size();
}

// Output formatting methods
std::string ElementEnumerationResult::getSummary() const {
    std::ostringstream oss;
    oss << "Element enumeration for window " << windowHandle;
    if (!windowTitle.empty()) {
        oss << " (\"" << windowTitle << "\")";
    }
    oss << ": ";

    if (success) {
        oss << "Found " << totalElementCount << " elements";
        if (filteredElementCount != totalElementCount) {
            oss << " (" << filteredElementCount << " after filtering)";
        }
        oss << " in " << enumerationTime.count() << "ms";

        if (!meetsPerformanceTarget()) {
            oss << " (exceeds 2000ms target)";
        }
    } else {
        oss << "Failed";
        if (!errorMessage.empty()) {
            oss << " - " << errorMessage;
        }
    }

    return oss.str();
}

std::string ElementEnumerationResult::toJson() const {
    std::ostringstream oss;
    oss << "{";
    oss << "\"windowHandle\":\"" << windowHandle << "\",";
    oss << "\"windowTitle\":\"" << windowTitle << "\",";
    oss << "\"success\":" << (success ? "true" : "false") << ",";
    oss << "\"totalElementCount\":" << totalElementCount << ",";
    oss << "\"filteredElementCount\":" << filteredElementCount << ",";
    oss << "\"enumerationTime\":" << enumerationTime.count() << ",";
    oss << "\"meetsPerformanceTarget\":" << (meetsPerformanceTarget() ? "true" : "false") << ",";
    oss << "\"hasAccessibilityPermissions\":" << (hasAccessibilityPermissions ? "true" : "false") << ",";
    oss << "\"supportsElementEnumeration\":" << (supportsElementEnumeration ? "true" : "false");

    if (!errorMessage.empty()) {
        oss << ",\"errorMessage\":\"" << errorMessage << "\"";
    }

    if (success && !elements.empty()) {
        oss << ",\"elements\":[";
        for (size_t i = 0; i < elements.size(); ++i) {
            if (i > 0) oss << ",";
            oss << elements[i].toJson();
        }
        oss << "]";
    }

    oss << "}";
    return oss.str();
}

std::string ElementEnumerationResult::getPerformanceInfo() const {
    std::ostringstream oss;
    oss << "Performance: " << enumerationTime.count() << "ms";
    oss << " (target: <2000ms) ";
    oss << (meetsPerformanceTarget() ? "✓" : "✗");
    oss << ", Elements: " << totalElementCount;
    if (filteredElementCount != totalElementCount) {
        oss << " (filtered: " << filteredElementCount << ")";
    }
    return oss.str();
}

} // namespace WindowManager