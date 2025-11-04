#pragma once

#include <string>
#include <chrono>
#include <vector>

namespace WindowManager {

// Forward declaration for workspace support
struct WorkspaceInfo;

/**
 * Window state enumeration for enhanced state tracking
 * Represents the current state of a window across workspaces
 */
enum class WindowState {
    Normal,     // Standard visible window
    Minimized,  // Hidden/iconified window
    Focused,    // Currently active window
    Hidden      // Hidden but not minimized (e.g., on different workspace)
};

/**
 * Core window information structure
 * Represents a single window with all necessary attributes for display and filtering
 */
struct WindowInfo {
    // Universal window identifier (platform-specific type erased to string)
    std::string handle;

    // Window display properties
    std::string title;
    int x, y;                    // Position coordinates (can be negative for multi-monitor)
    unsigned int width, height; // Dimensions (must be > 0 for valid windows)
    bool isVisible;

    // Process information
    unsigned int processId;      // Must be > 0 for valid processes
    std::string ownerName;       // Application/process name

    // NEW: Workspace information
    std::string workspaceId;              // Platform-specific workspace identifier
    std::string workspaceName;            // Human-readable workspace name
    bool isOnCurrentWorkspace;            // Quick check for current workspace

    // NEW: Enhanced state information
    WindowState state;                    // Comprehensive window state
    bool isFocused;                       // Currently focused window
    bool isMinimized;                     // Explicitly minimized state

    // NEW: Focus-specific capabilities (from data-model.md)
    std::chrono::steady_clock::time_point lastFocusTime;  // When window was last focused
    bool focusable;                       // Whether window can be programmatically focused
    bool requiresRestore;                 // Whether window needs restoration before focus
    bool workspaceSwitchRequired;         // Whether focusing requires workspace change

    // Default constructor
    WindowInfo();

    // Parameterized constructor (backward compatible)
    WindowInfo(const std::string& handle, const std::string& title,
               int x, int y, unsigned int width, unsigned int height,
               bool visible, unsigned int pid, const std::string& owner);

    // Enhanced constructor with workspace information
    WindowInfo(const std::string& handle, const std::string& title,
               int x, int y, unsigned int width, unsigned int height,
               bool visible, unsigned int pid, const std::string& owner,
               const std::string& workspaceId = "", const std::string& workspaceName = "",
               bool onCurrentWorkspace = true, WindowState state = WindowState::Normal);

    // Copy constructor and assignment operator
    WindowInfo(const WindowInfo& other) = default;
    WindowInfo& operator=(const WindowInfo& other) = default;

    // Move constructor and assignment operator
    WindowInfo(WindowInfo&& other) noexcept = default;
    WindowInfo& operator=(WindowInfo&& other) noexcept = default;

    // Destructor
    ~WindowInfo() = default;

    // Validation methods
    bool isValid() const;
    bool hasValidDimensions() const;
    bool hasValidPosition() const;
    bool hasWorkspaceInfo() const;        // NEW: Workspace information availability
    bool canBeFocused() const;            // NEW: Whether window is focusable
    bool needsWorkspaceSwitch() const;    // NEW: Whether workspace switching is required
    bool needsRestoration() const;        // NEW: Whether window needs restoration before focus

    // Display and formatting methods
    std::string toString() const;         // Enhanced with workspace info
    std::string toJson() const;           // Enhanced JSON with new fields
    std::string toShortString() const;    // NEW: Compact display format

    // T045: Workspace-aware JSON output formatting
    std::string toJsonWithWorkspaceContext(const std::vector<WorkspaceInfo>& workspaces) const;
    std::string toCompactJson() const;
    std::string getWorkspaceJsonFragment() const;

    // Comparison operators
    bool operator==(const WindowInfo& other) const;
    bool operator!=(const WindowInfo& other) const;

    // For sorting by title, position, or process
    bool operator<(const WindowInfo& other) const;
};

} // namespace WindowManager