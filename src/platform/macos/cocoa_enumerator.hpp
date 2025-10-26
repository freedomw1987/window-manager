#pragma once

#include "../../core/enumerator.hpp"
#include "platform_config.h"

#ifdef WM_PLATFORM_MACOS

#include <CoreGraphics/CoreGraphics.h>
#include <CoreFoundation/CoreFoundation.h>

namespace WindowManager {

/**
 * macOS-specific window enumerator using Core Graphics and Cocoa
 */
class CocoaEnumerator : public WindowEnumerator {
public:
    CocoaEnumerator();
    ~CocoaEnumerator() override = default;

    // WindowEnumerator interface implementation
    std::vector<WindowInfo> enumerateWindows() override;
    bool refreshWindowList() override;
    std::optional<WindowInfo> getWindowInfo(const std::string& handle) override;
    bool focusWindow(const std::string& handle) override;
    bool isWindowValid(const std::string& handle) override;

    // NEW: Workspace operations
    std::vector<WorkspaceInfo> enumerateWorkspaces() override;
    std::optional<WorkspaceInfo> getCurrentWorkspace() override;
    std::vector<WindowInfo> enumerateAllWorkspaceWindows() override;
    std::vector<WindowInfo> getWindowsOnWorkspace(const std::string& workspaceId) override;
    std::optional<WindowInfo> getEnhancedWindowInfo(const std::string& handle) override;
    bool isWorkspaceSupported() const override;
    std::optional<WindowInfo> getFocusedWindow() override;

    // Performance and diagnostics
    std::chrono::milliseconds getLastEnumerationTime() const override;
    size_t getWindowCount() const override;
    std::string getPlatformInfo() const override;

private:
    // Helper methods for Core Graphics/Cocoa API
    WindowInfo createWindowInfo(CGWindowID windowId, CFDictionaryRef windowInfo);
    std::string getWindowTitle(CFDictionaryRef windowInfo);
    std::string getProcessName(pid_t processId);
    bool isWindowVisible(CFDictionaryRef windowInfo);
    CGWindowID stringToHandle(const std::string& handleStr);
    std::string handleToString(CGWindowID windowId);
    bool checkAccessibilityPermissions() const;

    // NEW: Workspace/Spaces helper methods
    std::string getWindowWorkspaceId(CGWindowID windowId, CFDictionaryRef windowInfo);
    std::string getWorkspaceName(const std::string& workspaceId);
    bool isWindowOnCurrentWorkspace(CGWindowID windowId, CFDictionaryRef windowInfo);
    WindowState getWindowState(CGWindowID windowId, CFDictionaryRef windowInfo);
    std::optional<WindowInfo> getFocusedWindowViaAccessibility();

    // Core Graphics window list options
    static constexpr CGWindowListOption kWindowListOptions =
        kCGWindowListOptionAll | kCGWindowListExcludeDesktopElements;
};

} // namespace WindowManager

#endif // WM_PLATFORM_MACOS