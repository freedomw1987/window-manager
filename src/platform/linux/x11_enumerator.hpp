#pragma once

#include "../../core/enumerator.hpp"
#include "platform_config.h"

#ifdef WM_PLATFORM_LINUX

namespace WindowManager {

/**
 * Linux-specific window enumerator using X11 API
 */
class X11Enumerator : public WindowEnumerator {
public:
    X11Enumerator();
    ~X11Enumerator() override;

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
    // X11 display connection
    Display* display_;
    Window rootWindow_;

    // Helper methods for X11 API
    void initializeX11();
    void cleanupX11();
    WindowInfo createWindowInfo(Window window);
    std::string getWindowTitle(Window window);
    std::string getProcessName(unsigned long pid);
    void getWindowGeometry(Window window, int& x, int& y, unsigned int& width, unsigned int& height);
    unsigned long getWindowPid(Window window);
    bool isWindowVisible(Window window);
    void enumerateWindowsRecursive(Window window, std::vector<WindowInfo>& windows);
    Window stringToHandle(const std::string& handleStr);
    std::string handleToString(Window window);

    // EWMH (Extended Window Manager Hints) support
    Atom netWmNameAtom_;
    Atom netWmPidAtom_;
    Atom netWmStateAtom_;
    Atom netWmStateHiddenAtom_;
    bool ewmhSupported_;

    // NEW: Workspace/Desktop EWMH atoms
    Atom netNumberOfDesktopsAtom_;
    Atom netDesktopNamesAtom_;
    Atom netCurrentDesktopAtom_;
    Atom netWmDesktopAtom_;
    Atom netActiveWindowAtom_;

    void initializeEWMH();
    std::string getProperty(Window window, Atom property);
    unsigned long getPropertyLong(Window window, Atom property);

    // NEW: Workspace helper methods
    std::string getWindowWorkspaceId(Window window);
    std::string getWorkspaceName(const std::string& workspaceId);
    bool isWindowOnCurrentWorkspace(Window window);
    WindowState getWindowState(Window window);
    int getCurrentDesktopIndex();
    int getWindowDesktopIndex(Window window);
};

} // namespace WindowManager

#endif // WM_PLATFORM_LINUX