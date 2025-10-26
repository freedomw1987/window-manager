#include "win32_enumerator.hpp"
#include "../../core/exceptions.hpp"

#ifdef WM_PLATFORM_WINDOWS

#include <sstream>
#include <iostream>
#include <cstdlib>
#include <set>
#include <map>

namespace WindowManager {

// Static member initialization
std::vector<WindowInfo>* Win32Enumerator::s_windowList = nullptr;

Win32Enumerator::Win32Enumerator()
    : m_pVirtualDesktopManager(nullptr)
    , m_comInitialized(false)
    , m_virtualDesktopSupported(false) {
    // Initialize COM for Virtual Desktop Manager
    initializeVirtualDesktopManager();
}

Win32Enumerator::~Win32Enumerator() {
    cleanupVirtualDesktopManager();
}

std::vector<WindowInfo> Win32Enumerator::enumerateWindows() {
    auto start = std::chrono::steady_clock::now();

    std::vector<WindowInfo> windows;
    s_windowList = &windows;

    // Use EnumWindows to enumerate all top-level windows
    if (!EnumWindows(enumWindowsProc, 0)) {
        DWORD error = GetLastError();
        throw WindowEnumerationException("EnumWindows failed with error code: " + std::to_string(error));
    }

    s_windowList = nullptr;
    cachedWindows_ = windows;

    auto end = std::chrono::steady_clock::now();
    updateEnumerationTime(start, end);

    return windows;
}

bool Win32Enumerator::refreshWindowList() {
    try {
        enumerateWindows();
        return true;
    } catch (const WindowManagerException&) {
        return false;
    }
}

std::optional<WindowInfo> Win32Enumerator::getWindowInfo(const std::string& handle) {
    HWND hwnd = stringToHandle(handle);
    if (!IsWindow(hwnd)) {
        return std::nullopt;
    }

    try {
        return createWindowInfo(hwnd);
    } catch (const WindowManagerException&) {
        return std::nullopt;
    }
}

bool Win32Enumerator::focusWindow(const std::string& handle) {
    HWND hwnd = stringToHandle(handle);
    if (!IsWindow(hwnd)) {
        return false;
    }

    // Bring window to foreground
    if (!SetForegroundWindow(hwnd)) {
        return false;
    }

    // If minimized, restore it
    if (IsIconic(hwnd)) {
        ShowWindow(hwnd, SW_RESTORE);
    }

    return true;
}

bool Win32Enumerator::isWindowValid(const std::string& handle) {
    HWND hwnd = stringToHandle(handle);
    return IsWindow(hwnd) != FALSE;
}

std::chrono::milliseconds Win32Enumerator::getLastEnumerationTime() const {
    return lastEnumerationDuration_;
}

size_t Win32Enumerator::getWindowCount() const {
    return cachedWindows_.size();
}

std::string Win32Enumerator::getPlatformInfo() const {
    std::ostringstream oss;
    oss << "Windows Win32 API Enumerator";

    // Get Windows version info
    OSVERSIONINFOW osvi = {};
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOW);
    if (GetVersionExW(&osvi)) {
        oss << " (Windows " << osvi.dwMajorVersion << "." << osvi.dwMinorVersion << ")";
    }

    return oss.str();
}

// Static callback function for EnumWindows
BOOL CALLBACK Win32Enumerator::enumWindowsProc(HWND hwnd, LPARAM lParam) {
    if (!s_windowList) {
        return FALSE; // Stop enumeration if no list available
    }

    try {
        // Only include visible windows with titles
        if (IsWindowVisible(hwnd)) {
            char title[WM_MAX_WINDOW_TITLE_LENGTH];
            if (GetWindowTextA(hwnd, title, WM_MAX_WINDOW_TITLE_LENGTH) > 0) {
                Win32Enumerator enumerator;
                WindowInfo info = enumerator.createWindowInfo(hwnd);
                if (info.isValid()) {
                    s_windowList->push_back(info);
                }
            }
        }
    } catch (const WindowManagerException&) {
        // Continue enumeration even if one window fails
    }

    return TRUE; // Continue enumeration
}

WindowInfo Win32Enumerator::createWindowInfo(HWND hwnd) {
    WindowInfo info;

    // Set handle
    info.handle = handleToString(hwnd);

    // Get window title
    info.title = getWindowTitle(hwnd);

    // Get window position and size
    RECT rect;
    if (!GetWindowRect(hwnd, &rect)) {
        throw WindowOperationException("GetWindowRect", "Failed to get window rectangle");
    }

    info.x = rect.left;
    info.y = rect.top;
    info.width = static_cast<unsigned int>(rect.right - rect.left);
    info.height = static_cast<unsigned int>(rect.bottom - rect.top);

    // Get visibility state
    info.isVisible = IsWindowVisible(hwnd) != FALSE;

    // Get process information
    DWORD processId;
    GetWindowThreadProcessId(hwnd, &processId);
    info.processId = static_cast<unsigned int>(processId);
    info.ownerName = getProcessName(processId);

    // NEW: Add workspace information (T017)
    info.workspaceId = getWindowWorkspaceId(hwnd);
    info.workspaceName = getWorkspaceName(info.workspaceId);
    info.isOnCurrentWorkspace = isWindowOnCurrentWorkspace(hwnd);

    // NEW: Add enhanced state information (T018)
    info.state = getWindowState(hwnd);
    info.isFocused = (info.state == WindowState::Focused);
    info.isMinimized = (info.state == WindowState::Minimized);

    return info;
}

std::string Win32Enumerator::getWindowTitle(HWND hwnd) {
    char title[WM_MAX_WINDOW_TITLE_LENGTH];
    int length = GetWindowTextA(hwnd, title, WM_MAX_WINDOW_TITLE_LENGTH);
    return std::string(title, length);
}

std::string Win32Enumerator::getProcessName(DWORD processId) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
    if (!hProcess) {
        return "Unknown";
    }

    char processName[WM_MAX_PROCESS_NAME_LENGTH];
    DWORD size = WM_MAX_PROCESS_NAME_LENGTH;

    if (QueryFullProcessImageNameA(hProcess, 0, processName, &size)) {
        CloseHandle(hProcess);

        // Extract just the filename from the full path
        std::string fullPath(processName, size);
        size_t lastSlash = fullPath.find_last_of("\\/");
        if (lastSlash != std::string::npos) {
            return fullPath.substr(lastSlash + 1);
        }
        return fullPath;
    }

    CloseHandle(hProcess);
    return "Unknown";
}

HWND Win32Enumerator::stringToHandle(const std::string& handleStr) {
    try {
        uintptr_t handle = std::stoull(handleStr, nullptr, 16);
        return reinterpret_cast<HWND>(handle);
    } catch (const std::exception&) {
        return nullptr;
    }
}

std::string Win32Enumerator::handleToString(HWND hwnd) {
    uintptr_t handle = reinterpret_cast<uintptr_t>(hwnd);
    std::ostringstream oss;
    oss << std::hex << handle;
    return oss.str();
}

// Virtual Desktop Manager implementation
bool Win32Enumerator::initializeVirtualDesktopManager() {
    // Initialize COM
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (SUCCEEDED(hr)) {
        m_comInitialized = true;
    } else if (hr == RPC_E_CHANGED_MODE) {
        // COM already initialized with different threading model
        m_comInitialized = false; // Don't uninitialize
    } else {
        return false;
    }

    // Try to create Virtual Desktop Manager interface
    hr = CoCreateInstance(
        CLSID_VirtualDesktopManager,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_IVirtualDesktopManager,
        (void**)&m_pVirtualDesktopManager
    );

    if (SUCCEEDED(hr)) {
        m_virtualDesktopSupported = true;
        return true;
    } else {
        // Virtual Desktop not supported - fallback gracefully
        m_virtualDesktopSupported = false;
        return false;
    }
}

void Win32Enumerator::cleanupVirtualDesktopManager() {
    if (m_pVirtualDesktopManager) {
        m_pVirtualDesktopManager->Release();
        m_pVirtualDesktopManager = nullptr;
    }

    if (m_comInitialized) {
        CoUninitialize();
        m_comInitialized = false;
    }
}

bool Win32Enumerator::isWorkspaceSupported() const {
    return m_virtualDesktopSupported;
}

std::string Win32Enumerator::getWindowWorkspaceId(HWND hwnd) {
    if (!m_virtualDesktopSupported || !m_pVirtualDesktopManager) {
        return ""; // Return empty if not supported
    }

    GUID desktopId;
    HRESULT hr = m_pVirtualDesktopManager->GetWindowDesktopId(hwnd, &desktopId);
    if (SUCCEEDED(hr)) {
        // Convert GUID to string
        wchar_t guidStr[40];
        StringFromGUID2(desktopId, guidStr, 40);

        // Convert wchar_t to std::string
        char guidCharStr[40];
        wcstombs(guidCharStr, guidStr, 40);
        return std::string(guidCharStr);
    }

    return ""; // Return empty string on failure
}

std::string Win32Enumerator::getWorkspaceName(const std::string& workspaceId) {
    // For Windows, generate human-readable names for workspace IDs
    if (workspaceId.empty() || workspaceId == "default") {
        return "Desktop";
    }

    // Try to get a more meaningful name from the system
    // Since Windows doesn't provide user-defined names via public APIs,
    // we'll use a simple naming convention
    static std::map<std::string, int> workspaceIndexMap;
    static int nextIndex = 1;

    if (workspaceIndexMap.find(workspaceId) == workspaceIndexMap.end()) {
        workspaceIndexMap[workspaceId] = nextIndex++;
    }

    return "Desktop " + std::to_string(workspaceIndexMap[workspaceId]);
}

bool Win32Enumerator::isWindowOnCurrentWorkspace(HWND hwnd) {
    if (!m_virtualDesktopSupported || !m_pVirtualDesktopManager) {
        return true; // Assume yes if not supported
    }

    BOOL isOnCurrent = TRUE;
    HRESULT hr = m_pVirtualDesktopManager->IsWindowOnCurrentVirtualDesktop(hwnd, &isOnCurrent);
    return SUCCEEDED(hr) ? (isOnCurrent != FALSE) : true;
}

WindowState Win32Enumerator::getWindowState(HWND hwnd) {
    // Check if window is minimized
    if (IsIconic(hwnd)) {
        return WindowState::Minimized;
    }

    // Check if window is focused
    HWND foregroundWindow = GetForegroundWindow();
    if (foregroundWindow == hwnd) {
        return WindowState::Focused;
    }

    // Check if window is on current workspace
    if (!isWindowOnCurrentWorkspace(hwnd)) {
        return WindowState::Hidden;
    }

    return WindowState::Normal;
}

std::vector<WorkspaceInfo> Win32Enumerator::enumerateWorkspaces() {
    std::vector<WorkspaceInfo> workspaces;

    if (!m_virtualDesktopSupported) {
        // Create a single default workspace when Virtual Desktops not supported
        WorkspaceInfo defaultWorkspace("default", "Desktop", 0, true);
        workspaces.push_back(defaultWorkspace);
        return workspaces;
    }

    // T016: Enhanced workspace enumeration
    // Note: The public IVirtualDesktopManager interface provides limited enumeration.
    // We detect workspace IDs from actual windows and build the workspace list dynamically.
    std::set<std::string> detectedWorkspaceIds;
    std::string currentWorkspaceId;

    // First, enumerate all windows to detect workspaces
    std::vector<HWND> allWindows;
    EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
        auto* windows = reinterpret_cast<std::vector<HWND>*>(lParam);
        if (IsWindowVisible(hwnd)) {
            char title[256];
            if (GetWindowTextA(hwnd, title, 256) > 0) {
                windows->push_back(hwnd);
            }
        }
        return TRUE;
    }, reinterpret_cast<LPARAM>(&allWindows));

    // Detect unique workspace IDs from windows
    for (HWND hwnd : allWindows) {
        std::string workspaceId = getWindowWorkspaceId(hwnd);
        if (!workspaceId.empty()) {
            detectedWorkspaceIds.insert(workspaceId);
            if (isWindowOnCurrentWorkspace(hwnd) && currentWorkspaceId.empty()) {
                currentWorkspaceId = workspaceId;
            }
        }
    }

    // If no workspaces detected, create default
    if (detectedWorkspaceIds.empty()) {
        WorkspaceInfo defaultWorkspace("current", "Desktop 1", 0, true);
        workspaces.push_back(defaultWorkspace);
    } else {
        // Create WorkspaceInfo objects for detected workspaces
        int index = 0;
        for (const auto& workspaceId : detectedWorkspaceIds) {
            bool isCurrent = (workspaceId == currentWorkspaceId);
            std::string name = getWorkspaceName(workspaceId);
            WorkspaceInfo workspace(workspaceId, name, index++, isCurrent);
            workspaces.push_back(workspace);
        }
    }

    cachedWorkspaces_ = workspaces;
    return workspaces;
}

std::optional<WorkspaceInfo> Win32Enumerator::getCurrentWorkspace() {
    auto workspaces = enumerateWorkspaces();
    for (const auto& workspace : workspaces) {
        if (workspace.isCurrent) {
            return workspace;
        }
    }
    return std::nullopt;
}

std::vector<WindowInfo> Win32Enumerator::enumerateAllWorkspaceWindows() {
    // For Windows, this is the same as regular enumeration since
    // EnumWindows already enumerates across all virtual desktops
    return enumerateWindows();
}

std::vector<WindowInfo> Win32Enumerator::getWindowsOnWorkspace(const std::string& workspaceId) {
    auto allWindows = enumerateAllWorkspaceWindows();
    std::vector<WindowInfo> filteredWindows;

    for (const auto& window : allWindows) {
        if (window.workspaceId == workspaceId || workspaceId.empty()) {
            filteredWindows.push_back(window);
        }
    }

    return filteredWindows;
}

std::optional<WindowInfo> Win32Enumerator::getEnhancedWindowInfo(const std::string& handle) {
    HWND hwnd = stringToHandle(handle);
    if (!IsWindow(hwnd)) {
        return std::nullopt;
    }

    try {
        WindowInfo info = createWindowInfo(hwnd);

        // Add workspace information
        info.workspaceId = getWindowWorkspaceId(hwnd);
        info.workspaceName = "Desktop"; // Simple name for Windows
        info.isOnCurrentWorkspace = isWindowOnCurrentWorkspace(hwnd);
        info.state = getWindowState(hwnd);
        info.isFocused = (info.state == WindowState::Focused);
        info.isMinimized = (info.state == WindowState::Minimized);

        return info;
    } catch (const WindowManagerException&) {
        return std::nullopt;
    }
}

std::optional<WindowInfo> Win32Enumerator::getFocusedWindow() {
    HWND foregroundWindow = GetForegroundWindow();
    if (foregroundWindow == NULL) {
        return std::nullopt;
    }

    return getEnhancedWindowInfo(handleToString(foregroundWindow));
}

} // namespace WindowManager

#endif // WM_PLATFORM_WINDOWS