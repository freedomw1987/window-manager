#pragma once

#include "window.hpp"
#include "workspace.hpp"
#include <vector>
#include <memory>
#include <chrono>
#include <optional>

namespace WindowManager {

/**
 * Window enumeration interface
 * Abstract base class for platform-specific implementations
 */
class WindowEnumerator {
public:
    virtual ~WindowEnumerator() = default;

    // Core operations
    virtual std::vector<WindowInfo> enumerateWindows() = 0;
    virtual bool refreshWindowList() = 0;

    // Window-specific operations
    virtual std::optional<WindowInfo> getWindowInfo(const std::string& handle) = 0;
    virtual bool focusWindow(const std::string& handle) = 0;
    virtual bool isWindowValid(const std::string& handle) = 0;

    // NEW: Workspace operations
    virtual std::vector<WorkspaceInfo> enumerateWorkspaces() = 0;
    virtual std::optional<WorkspaceInfo> getCurrentWorkspace() = 0;
    virtual std::vector<WindowInfo> enumerateAllWorkspaceWindows() = 0;
    virtual std::vector<WindowInfo> getWindowsOnWorkspace(const std::string& workspaceId) = 0;
    virtual std::optional<WindowInfo> getEnhancedWindowInfo(const std::string& handle) = 0;
    virtual bool isWorkspaceSupported() const = 0;
    virtual std::optional<WindowInfo> getFocusedWindow() = 0;

    // NEW: Workspace switching operations (for cross-workspace focus)
    virtual bool switchToWorkspace(const std::string& workspaceId) = 0;
    virtual bool canSwitchWorkspaces() const = 0;

    // Performance and diagnostics
    virtual std::chrono::milliseconds getLastEnumerationTime() const = 0;
    virtual size_t getWindowCount() const = 0;
    virtual std::string getPlatformInfo() const = 0;

    // Factory method - implemented in enumerator.cpp
    static std::unique_ptr<WindowEnumerator> create();

protected:
    // Protected members for derived classes
    std::vector<WindowInfo> cachedWindows_;
    std::vector<WorkspaceInfo> cachedWorkspaces_;
    std::chrono::steady_clock::time_point lastEnumerationTime_;
    std::chrono::steady_clock::time_point lastWorkspaceEnumerationTime_;
    std::chrono::milliseconds lastEnumerationDuration_{0};
    std::chrono::milliseconds lastWorkspaceEnumerationDuration_{0};

    // Helper method for updating timing information
    void updateEnumerationTime(const std::chrono::steady_clock::time_point& start,
                              const std::chrono::steady_clock::time_point& end);
};

} // namespace WindowManager