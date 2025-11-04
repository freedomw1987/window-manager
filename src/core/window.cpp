#include "window.hpp"
#include "workspace.hpp"
#include <sstream>
#include <iomanip>

namespace WindowManager {

// Default constructor
WindowInfo::WindowInfo()
    : handle("")
    , title("")
    , x(0)
    , y(0)
    , width(0)
    , height(0)
    , isVisible(false)
    , processId(0)
    , ownerName("")
    , workspaceId("")
    , workspaceName("")
    , isOnCurrentWorkspace(true)
    , state(WindowState::Normal)
    , isFocused(false)
    , isMinimized(false)
{
}

// Parameterized constructor (backward compatible)
WindowInfo::WindowInfo(const std::string& handle, const std::string& title,
                       int x, int y, unsigned int width, unsigned int height,
                       bool visible, unsigned int pid, const std::string& owner)
    : handle(handle)
    , title(title)
    , x(x)
    , y(y)
    , width(width)
    , height(height)
    , isVisible(visible)
    , processId(pid)
    , ownerName(owner)
    , workspaceId("")
    , workspaceName("")
    , isOnCurrentWorkspace(true)
    , state(WindowState::Normal)
    , isFocused(false)
    , isMinimized(false)
{
}

// Enhanced constructor with workspace information
WindowInfo::WindowInfo(const std::string& handle, const std::string& title,
                       int x, int y, unsigned int width, unsigned int height,
                       bool visible, unsigned int pid, const std::string& owner,
                       const std::string& workspaceId, const std::string& workspaceName,
                       bool onCurrentWorkspace, WindowState state)
    : handle(handle)
    , title(title)
    , x(x)
    , y(y)
    , width(width)
    , height(height)
    , isVisible(visible)
    , processId(pid)
    , ownerName(owner)
    , workspaceId(workspaceId)
    , workspaceName(workspaceName)
    , isOnCurrentWorkspace(onCurrentWorkspace)
    , state(state)
    , isFocused(state == WindowState::Focused)
    , isMinimized(state == WindowState::Minimized)
{
}

// Validation methods
bool WindowInfo::isValid() const {
    return !handle.empty() &&
           hasValidDimensions() &&
           processId > 0 &&
           (workspaceId.empty() || workspaceId.length() > 0); // Workspace ID if present must be non-empty
}

bool WindowInfo::hasValidDimensions() const {
    return width > 0 && height > 0;
}

bool WindowInfo::hasValidPosition() const {
    // Position can be negative (off-screen or multi-monitor scenarios)
    // So we just check that both coordinates are not unreasonably large
    constexpr int MAX_REASONABLE_COORD = 100000;
    return x > -MAX_REASONABLE_COORD && x < MAX_REASONABLE_COORD &&
           y > -MAX_REASONABLE_COORD && y < MAX_REASONABLE_COORD;
}

bool WindowInfo::hasWorkspaceInfo() const {
    return !workspaceId.empty() || !workspaceName.empty();
}

// Display and formatting methods
std::string WindowInfo::toString() const {
    std::ostringstream oss;
    oss << "[" << processId << "] " << ownerName;
    if (!title.empty()) {
        oss << " - " << title;
    }

    // Add workspace information
    if (hasWorkspaceInfo()) {
        oss << "\n    Workspace: ";
        if (!workspaceName.empty()) {
            oss << workspaceName;
        } else {
            oss << "ID " << workspaceId;
        }
        if (!isOnCurrentWorkspace) {
            oss << " (not current)";
        }
    }

    oss << "\n    Position: (" << x << ", " << y << ")";
    oss << "  Size: " << width << "x" << height;

    // Add state information
    oss << "  State: ";
    switch (state) {
        case WindowState::Normal: oss << "Normal"; break;
        case WindowState::Minimized: oss << "Minimized"; break;
        case WindowState::Focused: oss << "Focused"; break;
        case WindowState::Hidden: oss << "Hidden"; break;
    }

    if (!isVisible) {
        oss << "  [Not Visible]";
    }

    return oss.str();
}

std::string WindowInfo::toJson() const {
    std::ostringstream oss;
    oss << "{\n";
    oss << "  \"handle\": \"" << handle << "\",\n";
    oss << "  \"title\": \"" << title << "\",\n";
    oss << "  \"x\": " << x << ", \"y\": " << y << ",\n";
    oss << "  \"width\": " << width << ", \"height\": " << height << ",\n";
    oss << "  \"isVisible\": " << (isVisible ? "true" : "false") << ",\n";
    oss << "  \"processId\": " << processId << ",\n";
    oss << "  \"ownerName\": \"" << ownerName << "\",\n";

    // NEW FIELDS (added to existing structure)
    oss << "  \"workspaceId\": \"" << workspaceId << "\",\n";
    oss << "  \"workspaceName\": \"" << workspaceName << "\",\n";
    oss << "  \"isOnCurrentWorkspace\": " << (isOnCurrentWorkspace ? "true" : "false") << ",\n";

    // Convert state enum to string
    std::string stateStr = "Normal";
    switch (state) {
        case WindowState::Normal: stateStr = "Normal"; break;
        case WindowState::Minimized: stateStr = "Minimized"; break;
        case WindowState::Focused: stateStr = "Focused"; break;
        case WindowState::Hidden: stateStr = "Hidden"; break;
    }
    oss << "  \"state\": \"" << stateStr << "\",\n";
    oss << "  \"isFocused\": " << (isFocused ? "true" : "false") << ",\n";
    oss << "  \"isMinimized\": " << (isMinimized ? "true" : "false") << "\n";
    oss << "}";
    return oss.str();
}

std::string WindowInfo::toShortString() const {
    std::ostringstream oss;
    oss << ownerName;
    if (!title.empty()) {
        oss << " - " << title;
    }
    oss << " [PID: " << processId << "]";

    if (hasWorkspaceInfo()) {
        if (!workspaceName.empty()) {
            oss << " [" << workspaceName << "]";
        } else if (!workspaceId.empty()) {
            oss << " [WS: " << workspaceId << "]";
        }
    }

    // Add state indicators
    if (isFocused) {
        oss << " [Focused]";
    } else if (isMinimized) {
        oss << " [Minimized]";
    } else if (!isOnCurrentWorkspace) {
        oss << " [Other Desktop]";
    }

    return oss.str();
}

// T045: Workspace-aware JSON output formatting

std::string WindowInfo::toJsonWithWorkspaceContext(const std::vector<WorkspaceInfo>& workspaces) const {
    std::ostringstream oss;
    oss << "{\n";
    oss << "  \"handle\": \"" << handle << "\",\n";
    oss << "  \"title\": \"" << title << "\",\n";
    oss << "  \"x\": " << x << ", \"y\": " << y << ",\n";
    oss << "  \"width\": " << width << ", \"height\": " << height << ",\n";
    oss << "  \"isVisible\": " << (isVisible ? "true" : "false") << ",\n";
    oss << "  \"processId\": " << processId << ",\n";
    oss << "  \"ownerName\": \"" << ownerName << "\",\n";

    // Enhanced workspace information with context
    oss << "  \"workspace\": {\n";
    oss << "    \"id\": \"" << workspaceId << "\",\n";
    oss << "    \"name\": \"" << workspaceName << "\",\n";
    oss << "    \"isCurrent\": " << (isOnCurrentWorkspace ? "true" : "false") << ",\n";

    // Find workspace details from context
    for (const auto& workspace : workspaces) {
        if (workspace.id == workspaceId) {
            oss << "    \"index\": " << workspace.index << ",\n";
            oss << "    \"isCurrentWorkspace\": " << (workspace.isCurrent ? "true" : "false") << ",\n";
            break;
        }
    }

    oss << "    \"hasWorkspaceInfo\": " << (hasWorkspaceInfo() ? "true" : "false") << "\n";
    oss << "  },\n";

    // Enhanced state information
    oss << "  \"state\": {\n";
    oss << "    \"current\": \"";
    switch (state) {
        case WindowState::Normal: oss << "Normal"; break;
        case WindowState::Minimized: oss << "Minimized"; break;
        case WindowState::Focused: oss << "Focused"; break;
        case WindowState::Hidden: oss << "Hidden"; break;
    }
    oss << "\",\n";
    oss << "    \"isFocused\": " << (isFocused ? "true" : "false") << ",\n";
    oss << "    \"isMinimized\": " << (isMinimized ? "true" : "false") << ",\n";
    oss << "    \"isHidden\": " << (!isVisible ? "true" : "false") << "\n";
    oss << "  },\n";

    // Position and size grouped
    oss << "  \"geometry\": {\n";
    oss << "    \"position\": {\"x\": " << x << ", \"y\": " << y << "},\n";
    oss << "    \"size\": {\"width\": " << width << ", \"height\": " << height << "},\n";
    oss << "    \"isValidPosition\": " << (hasValidPosition() ? "true" : "false") << ",\n";
    oss << "    \"isValidDimensions\": " << (hasValidDimensions() ? "true" : "false") << "\n";
    oss << "  },\n";

    // Metadata
    oss << "  \"metadata\": {\n";
    oss << "    \"isValid\": " << (isValid() ? "true" : "false") << "\n";
    oss << "  }\n";

    oss << "}";
    return oss.str();
}

std::string WindowInfo::toCompactJson() const {
    std::ostringstream oss;
    oss << "{";
    oss << "\"handle\":\"" << handle << "\",";
    oss << "\"title\":\"" << title << "\",";
    oss << "\"ownerName\":\"" << ownerName << "\",";
    oss << "\"processId\":" << processId << ",";
    oss << "\"workspaceId\":\"" << workspaceId << "\",";
    oss << "\"workspaceName\":\"" << workspaceName << "\",";
    oss << "\"state\":\"";

    switch (state) {
        case WindowState::Normal: oss << "Normal"; break;
        case WindowState::Minimized: oss << "Minimized"; break;
        case WindowState::Focused: oss << "Focused"; break;
        case WindowState::Hidden: oss << "Hidden"; break;
    }

    oss << "\",";
    oss << "\"isFocused\":" << (isFocused ? "true" : "false") << ",";
    oss << "\"isVisible\":" << (isVisible ? "true" : "false") << ",";
    oss << "\"isOnCurrentWorkspace\":" << (isOnCurrentWorkspace ? "true" : "false");
    oss << "}";
    return oss.str();
}

std::string WindowInfo::getWorkspaceJsonFragment() const {
    std::ostringstream oss;
    oss << "{\n";
    oss << "    \"id\": \"" << workspaceId << "\",\n";
    oss << "    \"name\": \"" << workspaceName << "\",\n";
    oss << "    \"isCurrent\": " << (isOnCurrentWorkspace ? "true" : "false") << ",\n";
    oss << "    \"hasInfo\": " << (hasWorkspaceInfo() ? "true" : "false") << "\n";
    oss << "  }";
    return oss.str();
}

// Comparison operators
bool WindowInfo::operator==(const WindowInfo& other) const {
    return handle == other.handle &&
           title == other.title &&
           x == other.x &&
           y == other.y &&
           width == other.width &&
           height == other.height &&
           isVisible == other.isVisible &&
           processId == other.processId &&
           ownerName == other.ownerName &&
           workspaceId == other.workspaceId &&
           workspaceName == other.workspaceName &&
           isOnCurrentWorkspace == other.isOnCurrentWorkspace &&
           state == other.state &&
           isFocused == other.isFocused &&
           isMinimized == other.isMinimized;
}

bool WindowInfo::operator!=(const WindowInfo& other) const {
    return !(*this == other);
}

bool WindowInfo::operator<(const WindowInfo& other) const {
    // Sort by title first, then by position
    if (title != other.title) {
        return title < other.title;
    }
    if (x != other.x) {
        return x < other.x;
    }
    return y < other.y;
}

} // namespace WindowManager