#include "workspace.hpp"
#include <sstream>

namespace WindowManager {

// Default constructor
WorkspaceInfo::WorkspaceInfo()
    : id(""), name(""), index(-1), isCurrent(false), platformData("") {
}

// Parameterized constructor
WorkspaceInfo::WorkspaceInfo(const std::string& id, const std::string& name,
                            int index, bool current)
    : id(id), name(name), index(index), isCurrent(current), platformData("") {
}

// Validation methods
bool WorkspaceInfo::isValid() const {
    return !id.empty() && index >= 0 && !name.empty();
}

// Display and formatting methods
std::string WorkspaceInfo::toString() const {
    std::stringstream ss;
    ss << "Workspace[" << index << "]: " << name
       << " (ID: " << id << ")";
    if (isCurrent) {
        ss << " [CURRENT]";
    }
    ss << " - " << windowHandles.size() << " windows";
    return ss.str();
}

std::string WorkspaceInfo::toJson() const {
    std::stringstream ss;
    ss << "{"
       << "\"id\":\"" << id << "\","
       << "\"name\":\"" << name << "\","
       << "\"index\":" << index << ","
       << "\"isCurrent\":" << (isCurrent ? "true" : "false") << ","
       << "\"windowCount\":" << windowHandles.size();

    if (!platformData.empty()) {
        ss << ",\"platformData\":\"" << platformData << "\"";
    }

    ss << "}";
    return ss.str();
}

// Comparison operators
bool WorkspaceInfo::operator==(const WorkspaceInfo& other) const {
    return id == other.id && index == other.index;
}

bool WorkspaceInfo::operator!=(const WorkspaceInfo& other) const {
    return !(*this == other);
}

bool WorkspaceInfo::operator<(const WorkspaceInfo& other) const {
    return index < other.index;
}

} // namespace WindowManager