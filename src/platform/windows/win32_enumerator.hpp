#pragma once

#include "../../core/enumerator.hpp"
#include "platform_config.h"

#ifdef WM_PLATFORM_WINDOWS

namespace WindowManager {

/**
 * Windows-specific window enumerator using Win32 API
 */
class Win32Enumerator : public WindowEnumerator {
public:
    Win32Enumerator();
    ~Win32Enumerator() override;

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

    // NEW: Workspace switching operations (for cross-workspace focus)
    bool switchToWorkspace(const std::string& workspaceId) override;
    bool canSwitchWorkspaces() const override;

    // Performance and diagnostics
    std::chrono::milliseconds getLastEnumerationTime() const override;
    size_t getWindowCount() const override;
    std::string getPlatformInfo() const override;

private:
    // Helper methods for Win32 API
    static BOOL CALLBACK enumWindowsProc(HWND hwnd, LPARAM lParam);
    WindowInfo createWindowInfo(HWND hwnd);
    std::string getWindowTitle(HWND hwnd);
    std::string getProcessName(DWORD processId);
    HWND stringToHandle(const std::string& handleStr);
    std::string handleToString(HWND hwnd);

    // NEW: Virtual Desktop/Workspace methods
    bool initializeVirtualDesktopManager();
    void cleanupVirtualDesktopManager();
    std::string getWindowWorkspaceId(HWND hwnd);
    std::string getWorkspaceName(const std::string& workspaceId);
    bool isWindowOnCurrentWorkspace(HWND hwnd);
    WindowState getWindowState(HWND hwnd);

    // COM interface for Virtual Desktop Manager
    IVirtualDesktopManager* m_pVirtualDesktopManager;
    bool m_comInitialized;
    bool m_virtualDesktopSupported;

    // Static helper for enumeration callback
    static std::vector<WindowInfo>* s_windowList;
};

} // namespace WindowManager

#endif // WM_PLATFORM_WINDOWS