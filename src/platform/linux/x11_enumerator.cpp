#include "x11_enumerator.hpp"
#include "../../core/exceptions.hpp"

#ifdef WM_PLATFORM_LINUX

#include <sstream>
#include <fstream>
#include <cstring>

namespace WindowManager {

X11Enumerator::X11Enumerator()
    : display_(nullptr)
    , rootWindow_(0)
    , netWmNameAtom_(0)
    , netWmPidAtom_(0)
    , netWmStateAtom_(0)
    , netWmStateHiddenAtom_(0)
    , ewmhSupported_(false)
    , netNumberOfDesktopsAtom_(0)
    , netDesktopNamesAtom_(0)
    , netCurrentDesktopAtom_(0)
    , netWmDesktopAtom_(0)
    , netActiveWindowAtom_(0) {
    initializeX11();
    initializeEWMH();
}

X11Enumerator::~X11Enumerator() {
    cleanupX11();
}

void X11Enumerator::initializeX11() {
    display_ = XOpenDisplay(nullptr);
    if (!display_) {
        throw WindowEnumerationException("Unable to open X11 display. Check DISPLAY environment variable.");
    }

    rootWindow_ = DefaultRootWindow(display_);
    if (!rootWindow_) {
        throw WindowEnumerationException("Unable to get root window from X11 display");
    }
}

void X11Enumerator::cleanupX11() {
    if (display_) {
        XCloseDisplay(display_);
        display_ = nullptr;
    }
}

void X11Enumerator::initializeEWMH() {
    // T024: Initialize Extended Window Manager Hints atoms and X11 connection
    netWmNameAtom_ = XInternAtom(display_, "_NET_WM_NAME", False);
    netWmPidAtom_ = XInternAtom(display_, "_NET_WM_PID", False);
    netWmStateAtom_ = XInternAtom(display_, "_NET_WM_STATE", False);
    netWmStateHiddenAtom_ = XInternAtom(display_, "_NET_WM_STATE_HIDDEN", False);

    // NEW: Workspace/Desktop EWMH atoms
    netNumberOfDesktopsAtom_ = XInternAtom(display_, "_NET_NUMBER_OF_DESKTOPS", False);
    netDesktopNamesAtom_ = XInternAtom(display_, "_NET_DESKTOP_NAMES", False);
    netCurrentDesktopAtom_ = XInternAtom(display_, "_NET_CURRENT_DESKTOP", False);
    netWmDesktopAtom_ = XInternAtom(display_, "_NET_WM_DESKTOP", False);
    netActiveWindowAtom_ = XInternAtom(display_, "_NET_ACTIVE_WINDOW", False);

    // Check if window manager supports EWMH
    Atom supportedAtom = XInternAtom(display_, "_NET_SUPPORTED", False);
    ewmhSupported_ = (supportedAtom != None);
}

std::vector<WindowInfo> X11Enumerator::enumerateWindows() {
    auto start = std::chrono::steady_clock::now();

    std::vector<WindowInfo> windows;

    try {
        enumerateWindowsRecursive(rootWindow_, windows);
    } catch (const std::exception& e) {
        throw WindowEnumerationException("X11 enumeration failed: " + std::string(e.what()));
    }

    cachedWindows_ = windows;

    auto end = std::chrono::steady_clock::now();
    updateEnumerationTime(start, end);

    return windows;
}

void X11Enumerator::enumerateWindowsRecursive(Window window, std::vector<WindowInfo>& windows) {
    Window root, parent;
    Window* children;
    unsigned int nchildren;

    if (XQueryTree(display_, window, &root, &parent, &children, &nchildren) == 0) {
        return; // Failed to query this window
    }

    // Process current window if it's not the root window
    if (window != rootWindow_) {
        try {
            WindowInfo info = createWindowInfo(window);
            if (info.isValid() && !info.title.empty()) {
                windows.push_back(info);
            }
        } catch (const WindowManagerException&) {
            // Continue with other windows even if one fails
        }
    }

    // Recursively process children
    for (unsigned int i = 0; i < nchildren; ++i) {
        enumerateWindowsRecursive(children[i], windows);
    }

    if (children) {
        XFree(children);
    }
}

bool X11Enumerator::refreshWindowList() {
    try {
        enumerateWindows();
        return true;
    } catch (const WindowManagerException&) {
        return false;
    }
}

std::optional<WindowInfo> X11Enumerator::getWindowInfo(const std::string& handle) {
    Window window = stringToHandle(handle);
    if (window == 0) {
        return std::nullopt;
    }

    try {
        return createWindowInfo(window);
    } catch (const WindowManagerException&) {
        return std::nullopt;
    }
}

bool X11Enumerator::focusWindow(const std::string& handle) {
    Window window = stringToHandle(handle);
    if (window == 0) {
        return false;
    }

    // Enhanced for User Story 2: Check if cross-workspace switching is needed
    if (ewmhSupported_) {
        // Get the window's desktop index
        int windowDesktop = getWindowDesktopIndex(window);
        int currentDesktop = getCurrentDesktopIndex();

        if (windowDesktop >= 0 && currentDesktop >= 0 && windowDesktop != currentDesktop) {
            // Window is on a different desktop - attempt to switch
            std::string targetWorkspaceId = std::to_string(windowDesktop);
            if (switchToWorkspace(targetWorkspaceId)) {
                // Wait for workspace switch to complete
                usleep(200000); // 200ms wait
            }
        }
    }

    // Raise the window and give it focus
    XRaiseWindow(display_, window);
    XSetInputFocus(display_, window, RevertToPointerRoot, CurrentTime);
    XFlush(display_);

    return true;
}

bool X11Enumerator::isWindowValid(const std::string& handle) {
    // Enhanced comprehensive handle validation for User Story 3

    // First check: handle format validation (comprehensive)
    if (handle.empty()) {
        return false;
    }

    // Check for valid hexadecimal format
    if (handle.length() > 16) { // 64-bit pointer max length
        return false;
    }

    // Validate hexadecimal characters
    for (char c : handle) {
        if (!std::isxdigit(c)) {
            return false;
        }
    }

    // Convert and validate handle value
    Window window = stringToHandle(handle);
    if (window == 0 || window == None) {
        return false;
    }

    // Check for obviously invalid window IDs (X11 Window IDs are typically > 0x100000)
    if (window < 0x100000) {
        return false;
    }

    // Verify display connection is available
    if (!display_) {
        return false;
    }

    // Second check: verify window still exists in X11 tree
    Window root, parent;
    Window* children = nullptr;
    unsigned int nchildren;

    // Set error handler to catch BadWindow errors
    XErrorHandler oldHandler = XSetErrorHandler([](Display*, XErrorEvent* error) -> int {
        return (error->error_code == BadWindow) ? 0 : 1;
    });

    int queryResult = XQueryTree(display_, window, &root, &parent, &children, &nchildren);
    if (children) {
        XFree(children);
    }

    XSetErrorHandler(oldHandler); // Restore original error handler

    if (queryResult == 0) {
        return false; // Window doesn't exist in X11 hierarchy
    }

    // Third check: verify window attributes are accessible
    XWindowAttributes attrs;
    if (XGetWindowAttributes(display_, window, &attrs) == 0) {
        return false; // Cannot access window attributes
    }

    // Fourth check: verify window has reasonable dimensions
    if (attrs.width <= 0 || attrs.height <= 0 ||
        attrs.width > 10000 || attrs.height > 10000) {
        return false; // Invalid window dimensions
    }

    // Fifth check: verify window position is reasonable (not massively off-screen)
    if (attrs.x < -5000 || attrs.x > 5000 || attrs.y < -5000 || attrs.y > 5000) {
        return false; // Window too far off-screen
    }

    // Sixth check: verify window class (not InputOnly)
    if (attrs.class == InputOnly) {
        return false; // InputOnly windows are not user windows
    }

    // Seventh check: verify window depth is reasonable
    if (attrs.depth == 0 || attrs.depth > 32) {
        return false; // Invalid color depth
    }

    // Eighth check: verify window is not completely unmapped and forgotten
    if (attrs.map_state == IsUnviewable) {
        // Check if window is still valid by trying to get its properties
        Atom actualType;
        int actualFormat;
        unsigned long nItems, bytesAfter;
        unsigned char* prop = nullptr;

        // Try to get WM_NAME property to verify window is still managed
        int result = XGetWindowProperty(display_, window, XA_WM_NAME, 0, 1,
                                       False, XA_STRING, &actualType, &actualFormat,
                                       &nItems, &bytesAfter, &prop);

        if (prop) XFree(prop);

        if (result != Success) {
            return false; // Window properties are not accessible
        }
    }

    // Ninth check: verify window is not a root window or desktop
    if (window == DefaultRootWindow(display_) || window == rootWindow_) {
        return false; // Root window is not a valid target
    }

    // Tenth check: verify window is not a subwindow of root (direct child)
    if (parent == rootWindow_ && attrs.class == InputOutput) {
        // This might be a top-level window, which is good
        // But verify it's not a window manager decoration or panel

        // Check if window has WM_CLASS property (indicates it's an application window)
        XClassHint classHint;
        if (XGetClassHint(display_, window, &classHint) == Success) {
            if (classHint.res_class) XFree(classHint.res_class);
            if (classHint.res_name) XFree(classHint.res_name);
            // Having WM_CLASS is a good sign it's a real application window
        } else {
            // No WM_CLASS might indicate a decorator or system window
            return false;
        }
    }

    // Eleventh check: verify window is not a override-redirect window (tooltips, menus, etc.)
    if (attrs.override_redirect == True) {
        return false; // Override-redirect windows are typically not user-focusable
    }

    // Twelfth check: verify window backing store is reasonable
    if (attrs.backing_store != NotUseful && attrs.backing_store != WhenMapped && attrs.backing_store != Always) {
        return false; // Invalid backing store value
    }

    return true;
}

std::chrono::milliseconds X11Enumerator::getLastEnumerationTime() const {
    return lastEnumerationDuration_;
}

size_t X11Enumerator::getWindowCount() const {
    return cachedWindows_.size();
}

std::string X11Enumerator::getPlatformInfo() const {
    std::ostringstream oss;
    oss << "Linux X11 Enumerator";

    if (display_) {
        oss << " (Display: " << DisplayString(display_) << ")";
        oss << " (Screen: " << DefaultScreen(display_) << ")";
    }

    if (ewmhSupported_) {
        oss << " [EWMH supported]";
    }

    return oss.str();
}

WindowInfo X11Enumerator::createWindowInfo(Window window) {
    WindowInfo info;

    // Set handle
    info.handle = handleToString(window);

    // Get window title
    info.title = getWindowTitle(window);

    // Get window geometry
    getWindowGeometry(window, info.x, info.y, info.width, info.height);

    // Get visibility state
    info.isVisible = isWindowVisible(window);

    // Get process information
    unsigned long pid = getWindowPid(window);
    info.processId = static_cast<unsigned int>(pid);
    info.ownerName = getProcessName(pid);

    // NEW: Add workspace information (T027)
    info.workspaceId = getWindowWorkspaceId(window);
    info.workspaceName = getWorkspaceName(info.workspaceId);
    info.isOnCurrentWorkspace = isWindowOnCurrentWorkspace(window);

    // NEW: Add enhanced state information (T028)
    info.state = getWindowState(window);
    info.isFocused = (info.state == WindowState::Focused);
    info.isMinimized = (info.state == WindowState::Minimized);

    return info;
}

std::string X11Enumerator::getWindowTitle(Window window) {
    // Try EWMH _NET_WM_NAME first (UTF-8)
    if (ewmhSupported_) {
        std::string title = getProperty(window, netWmNameAtom_);
        if (!title.empty()) {
            return title;
        }
    }

    // Fall back to standard WM_NAME
    char* windowName = nullptr;
    if (XFetchName(display_, window, &windowName) && windowName) {
        std::string title(windowName);
        XFree(windowName);
        return title;
    }

    return "";
}

std::string X11Enumerator::getProcessName(unsigned long pid) {
    if (pid == 0) {
        return "Unknown";
    }

    std::string procPath = "/proc/" + std::to_string(pid) + "/comm";
    std::ifstream procFile(procPath);

    if (procFile.is_open()) {
        std::string processName;
        std::getline(procFile, processName);
        return processName.empty() ? "Unknown" : processName;
    }

    return "Unknown";
}

void X11Enumerator::getWindowGeometry(Window window, int& x, int& y, unsigned int& width, unsigned int& height) {
    Window root;
    int border_width, depth;

    if (XGetGeometry(display_, window, &root, &x, &y, &width, &height,
                     reinterpret_cast<unsigned int*>(&border_width), reinterpret_cast<unsigned int*>(&depth)) == 0) {
        throw WindowOperationException("XGetGeometry", "Failed to get window geometry");
    }

    // Convert to screen coordinates
    Window child;
    XTranslateCoordinates(display_, window, root, 0, 0, &x, &y, &child);
}

unsigned long X11Enumerator::getWindowPid(Window window) {
    if (ewmhSupported_) {
        return getPropertyLong(window, netWmPidAtom_);
    }
    return 0;
}

bool X11Enumerator::isWindowVisible(Window window) {
    XWindowAttributes attrs;
    if (XGetWindowAttributes(display_, window, &attrs) == 0) {
        return false;
    }

    return attrs.map_state == IsViewable;
}

std::string X11Enumerator::getProperty(Window window, Atom property) {
    Atom actual_type;
    int actual_format;
    unsigned long nitems, bytes_after;
    unsigned char* prop = nullptr;

    if (XGetWindowProperty(display_, window, property, 0, (~0L), False,
                          AnyPropertyType, &actual_type, &actual_format,
                          &nitems, &bytes_after, &prop) != Success || !prop) {
        return "";
    }

    std::string result(reinterpret_cast<char*>(prop), nitems);
    XFree(prop);
    return result;
}

unsigned long X11Enumerator::getPropertyLong(Window window, Atom property) {
    Atom actual_type;
    int actual_format;
    unsigned long nitems, bytes_after;
    unsigned char* prop = nullptr;

    if (XGetWindowProperty(display_, window, property, 0, 1, False,
                          XA_CARDINAL, &actual_type, &actual_format,
                          &nitems, &bytes_after, &prop) != Success || !prop) {
        return 0;
    }

    unsigned long result = 0;
    if (nitems > 0 && actual_format == 32) {
        result = *reinterpret_cast<unsigned long*>(prop);
    }

    XFree(prop);
    return result;
}

Window X11Enumerator::stringToHandle(const std::string& handleStr) {
    try {
        return static_cast<Window>(std::stoull(handleStr, nullptr, 16));
    } catch (const std::exception&) {
        return 0;
    }
}

std::string X11Enumerator::handleToString(Window window) {
    std::ostringstream oss;
    oss << std::hex << window;
    return oss.str();
}

// NEW: Workspace/EWMH implementation (T024-T028)

std::vector<WorkspaceInfo> X11Enumerator::enumerateWorkspaces() {
    // T025: EWMH workspace enumeration using _NET_NUMBER_OF_DESKTOPS
    std::vector<WorkspaceInfo> workspaces;

    if (!ewmhSupported_) {
        // Create default workspace when EWMH not supported
        WorkspaceInfo defaultWorkspace("0", "Desktop", 0, true);
        workspaces.push_back(defaultWorkspace);
        cachedWorkspaces_ = workspaces;
        return workspaces;
    }

    // Get number of desktops
    unsigned long numDesktops = getPropertyLong(rootWindow_, netNumberOfDesktopsAtom_);
    if (numDesktops == 0) {
        numDesktops = 1; // Fallback to single desktop
    }

    // Get current desktop index
    int currentDesktop = getCurrentDesktopIndex();

    // Get desktop names if available
    std::string desktopNames = getProperty(rootWindow_, netDesktopNamesAtom_);
    std::vector<std::string> names;

    // Parse null-separated desktop names
    if (!desktopNames.empty()) {
        size_t start = 0;
        for (size_t i = 0; i < desktopNames.length(); ++i) {
            if (desktopNames[i] == '\0') {
                if (i > start) {
                    names.push_back(desktopNames.substr(start, i - start));
                }
                start = i + 1;
            }
        }
        // Add last name if no trailing null
        if (start < desktopNames.length()) {
            names.push_back(desktopNames.substr(start));
        }
    }

    // Create WorkspaceInfo objects
    for (unsigned long i = 0; i < numDesktops; ++i) {
        std::string id = std::to_string(i);
        std::string name = (i < names.size() && !names[i].empty())
                         ? names[i]
                         : "Desktop " + std::to_string(i + 1);
        bool isCurrent = (static_cast<int>(i) == currentDesktop);

        WorkspaceInfo workspace(id, name, static_cast<int>(i), isCurrent);
        workspaces.push_back(workspace);
    }

    cachedWorkspaces_ = workspaces;
    return workspaces;
}

std::optional<WorkspaceInfo> X11Enumerator::getCurrentWorkspace() {
    auto workspaces = enumerateWorkspaces();
    for (const auto& workspace : workspaces) {
        if (workspace.isCurrent) {
            return workspace;
        }
    }
    return std::nullopt;
}

std::vector<WindowInfo> X11Enumerator::enumerateAllWorkspaceWindows() {
    // Linux X11 enumeration already includes all workspaces
    return enumerateWindows();
}

std::vector<WindowInfo> X11Enumerator::getWindowsOnWorkspace(const std::string& workspaceId) {
    auto allWindows = enumerateAllWorkspaceWindows();
    std::vector<WindowInfo> filteredWindows;

    for (const auto& window : allWindows) {
        if (window.workspaceId == workspaceId || workspaceId.empty()) {
            filteredWindows.push_back(window);
        }
    }

    return filteredWindows;
}

std::optional<WindowInfo> X11Enumerator::getEnhancedWindowInfo(const std::string& handle) {
    Window window = stringToHandle(handle);
    if (window == 0) {
        return std::nullopt;
    }

    try {
        return createWindowInfo(window);
    } catch (const WindowManagerException&) {
        return std::nullopt;
    }
}

bool X11Enumerator::isWorkspaceSupported() const {
    return ewmhSupported_;
}

std::optional<WindowInfo> X11Enumerator::getFocusedWindow() {
    // T028: Focus detection via EWMH _NET_ACTIVE_WINDOW
    if (!ewmhSupported_) {
        return std::nullopt;
    }

    // Get active window from EWMH
    Window activeWindow = static_cast<Window>(getPropertyLong(rootWindow_, netActiveWindowAtom_));
    if (activeWindow != 0) {
        try {
            return createWindowInfo(activeWindow);
        } catch (const WindowManagerException&) {
            // Fall through to XGetInputFocus
        }
    }

    // Fallback to XGetInputFocus
    Window focusedWindow;
    int revertTo;
    XGetInputFocus(display_, &focusedWindow, &revertTo);

    if (focusedWindow != None && focusedWindow != PointerRoot) {
        try {
            return createWindowInfo(focusedWindow);
        } catch (const WindowManagerException&) {
            // Focus window might not be a regular window
        }
    }

    return std::nullopt;
}

// Helper methods implementation

std::string X11Enumerator::getWindowWorkspaceId(Window window) {
    // T026: Linux workspace detection using _NET_WM_DESKTOP
    if (!ewmhSupported_) {
        return "0"; // Default workspace
    }

    long desktop = static_cast<long>(getPropertyLong(window, netWmDesktopAtom_));

    // Special value 0xFFFFFFFF means window is on all desktops
    if (desktop == 0xFFFFFFFF) {
        return "all";
    }

    // Negative values or very large values are invalid
    if (desktop < 0 || desktop > 100) {
        return "0";
    }

    return std::to_string(desktop);
}

std::string X11Enumerator::getWorkspaceName(const std::string& workspaceId) {
    if (workspaceId == "all") {
        return "All Desktops";
    }

    if (workspaceId.empty()) {
        return "Desktop";
    }

    try {
        int index = std::stoi(workspaceId);

        // Try to get actual desktop name from EWMH
        if (ewmhSupported_) {
            std::string desktopNames = getProperty(rootWindow_, netDesktopNamesAtom_);
            if (!desktopNames.empty()) {
                std::vector<std::string> names;
                size_t start = 0;
                for (size_t i = 0; i < desktopNames.length(); ++i) {
                    if (desktopNames[i] == '\0') {
                        if (i > start) {
                            names.push_back(desktopNames.substr(start, i - start));
                        }
                        start = i + 1;
                    }
                }
                if (start < desktopNames.length()) {
                    names.push_back(desktopNames.substr(start));
                }

                if (index >= 0 && index < static_cast<int>(names.size()) && !names[index].empty()) {
                    return names[index];
                }
            }
        }

        return "Desktop " + std::to_string(index + 1);
    } catch (const std::exception&) {
        return "Desktop";
    }
}

bool X11Enumerator::isWindowOnCurrentWorkspace(Window window) {
    if (!ewmhSupported_) {
        return true; // Assume yes if EWMH not supported
    }

    int currentDesktop = getCurrentDesktopIndex();
    int windowDesktop = getWindowDesktopIndex(window);

    // Window on all desktops is always on current workspace
    if (windowDesktop == 0xFFFFFFFF) {
        return true;
    }

    return windowDesktop == currentDesktop;
}

WindowState X11Enumerator::getWindowState(Window window) {
    // Check if window is hidden/minimized via EWMH
    if (ewmhSupported_) {
        std::string state = getProperty(window, netWmStateAtom_);
        if (state.find("_NET_WM_STATE_HIDDEN") != std::string::npos) {
            return WindowState::Minimized;
        }
    }

    // Check if window is the active window
    if (ewmhSupported_) {
        Window activeWindow = static_cast<Window>(getPropertyLong(rootWindow_, netActiveWindowAtom_));
        if (activeWindow == window) {
            return WindowState::Focused;
        }
    }

    // Check if window is on current workspace
    if (!isWindowOnCurrentWorkspace(window)) {
        return WindowState::Hidden;
    }

    return WindowState::Normal;
}

int X11Enumerator::getCurrentDesktopIndex() {
    if (!ewmhSupported_) {
        return 0;
    }

    long current = static_cast<long>(getPropertyLong(rootWindow_, netCurrentDesktopAtom_));
    return (current >= 0) ? static_cast<int>(current) : 0;
}

int X11Enumerator::getWindowDesktopIndex(Window window) {
    if (!ewmhSupported_) {
        return 0;
    }

    long desktop = static_cast<long>(getPropertyLong(window, netWmDesktopAtom_));
    return static_cast<int>(desktop);
}

// NEW: Workspace switching operations (for cross-workspace focus)
bool X11Enumerator::switchToWorkspace(const std::string& workspaceId) {
    // Implementation for User Story 2 - Linux X11 workspace switching

    if (!ewmhSupported_ || !display_) {
        return false; // EWMH not supported or no display connection
    }

    // Convert workspace ID to integer
    int workspaceIndex;
    try {
        workspaceIndex = std::stoi(workspaceId);
    } catch (const std::exception&) {
        return false; // Invalid workspace ID format
    }

    // Validate workspace index
    if (workspaceIndex < 0) {
        return false;
    }

    try {
        // Create a client message to switch desktop
        XEvent event;
        event.type = ClientMessage;
        event.xclient.window = rootWindow_;
        event.xclient.message_type = netCurrentDesktopAtom_;
        event.xclient.format = 32;
        event.xclient.data.l[0] = workspaceIndex;
        event.xclient.data.l[1] = CurrentTime;
        event.xclient.data.l[2] = 0;
        event.xclient.data.l[3] = 0;
        event.xclient.data.l[4] = 0;

        // Send the message to the window manager
        int result = XSendEvent(display_, rootWindow_, False,
                               SubstructureRedirectMask | SubstructureNotifyMask,
                               &event);

        if (result != 0) {
            XFlush(display_);
            return true;
        }

        return false;
    } catch (...) {
        return false;
    }
}

bool X11Enumerator::canSwitchWorkspaces() const {
    // Check if EWMH workspace switching is supported
    // This requires EWMH support and a compatible window manager
    return ewmhSupported_ && display_ != nullptr;
}

} // namespace WindowManager

#endif // WM_PLATFORM_LINUX