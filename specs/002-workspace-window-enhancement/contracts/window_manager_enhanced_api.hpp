#pragma once

#include <vector>
#include <string>
#include <optional>
#include <chrono>
#include <memory>

/**
 * Enhanced Window Manager API Contract
 * Extends existing window management with workspace and enhanced state support
 *
 * Version: 2.0.0
 * Feature: Workspace Window Enhancement
 * Backward Compatibility: Full compatibility with existing 1.x API
 */

namespace WindowManager {

// Forward declarations
struct WindowInfo;
struct WorkspaceInfo;
struct SearchQuery;
struct FilterResult;

enum class WindowState {
    Normal,     // Standard visible window
    Minimized,  // Hidden/iconified window
    Focused,    // Currently active window
    Hidden      // Hidden but not minimized (e.g., on different workspace)
};

enum class SearchField {
    Title,      // Search in window title
    Owner,      // Search in application/owner name
    Both        // Search in both title and owner (default)
};

/**
 * Enhanced Window Enumerator Interface
 * Extends existing WindowEnumerator with workspace capabilities
 */
class EnhancedWindowEnumerator {
public:
    virtual ~EnhancedWindowEnumerator() = default;

    // EXISTING API (preserved for backward compatibility)
    virtual std::vector<WindowInfo> enumerateWindows() = 0;
    virtual bool refreshWindowList() = 0;
    virtual std::optional<WindowInfo> getWindowInfo(const std::string& handle) = 0;
    virtual bool focusWindow(const std::string& handle) = 0;
    virtual bool isWindowValid(const std::string& handle) = 0;
    virtual std::chrono::milliseconds getLastEnumerationTime() const = 0;
    virtual size_t getWindowCount() const = 0;
    virtual std::string getPlatformInfo() const = 0;

    // NEW WORKSPACE API

    /**
     * Enumerate all available workspaces/virtual desktops
     * @return Vector of WorkspaceInfo objects, empty if workspaces not supported
     */
    virtual std::vector<WorkspaceInfo> enumerateWorkspaces() = 0;

    /**
     * Get currently active workspace
     * @return Current workspace info, nullopt if not supported
     */
    virtual std::optional<WorkspaceInfo> getCurrentWorkspace() = 0;

    /**
     * Get all windows across all workspaces with enhanced state information
     * @return Vector of WindowInfo with workspace and state data
     */
    virtual std::vector<WindowInfo> enumerateAllWorkspaceWindows() = 0;

    /**
     * Get windows on a specific workspace
     * @param workspaceId Platform-specific workspace identifier
     * @return Vector of windows on the specified workspace
     */
    virtual std::vector<WindowInfo> getWindowsOnWorkspace(const std::string& workspaceId) = 0;

    /**
     * Get enhanced window information including workspace and state
     * @param handle Window handle
     * @return Enhanced window info, nullopt if window not found
     */
    virtual std::optional<WindowInfo> getEnhancedWindowInfo(const std::string& handle) = 0;

    /**
     * Check if workspace functionality is supported on current platform
     * @return true if workspace enumeration is available
     */
    virtual bool isWorkspaceSupported() const = 0;

    /**
     * Get currently focused window across all workspaces
     * @return Focused window info, nullopt if none focused or detection failed
     */
    virtual std::optional<WindowInfo> getFocusedWindow() = 0;

    // ENHANCED SEARCH API

    /**
     * Search windows using enhanced query supporting title and owner fields
     * @param query Search query with field specification
     * @return Filter result with matched windows and statistics
     */
    virtual FilterResult searchWindows(const SearchQuery& query) = 0;

    /**
     * Search windows by string (backward compatible)
     * @param searchTerm Search term to match against title and owner
     * @param caseSensitive Whether search should be case sensitive
     * @return Vector of matched windows
     */
    virtual std::vector<WindowInfo> searchWindows(const std::string& searchTerm,
                                                  bool caseSensitive = false) = 0;
};

/**
 * Enhanced Window Manager Interface
 * Main interface for workspace-aware window management
 */
class EnhancedWindowManager {
public:
    virtual ~EnhancedWindowManager() = default;

    /**
     * Initialize the window manager with platform detection
     * @return true if initialization successful
     */
    virtual bool initialize() = 0;

    /**
     * Get the platform-specific enumerator
     * @return Shared pointer to enumerator, nullptr if not initialized
     */
    virtual std::shared_ptr<EnhancedWindowEnumerator> getEnumerator() = 0;

    /**
     * Check if current platform supports workspace functionality
     * @return true if workspace features are available
     */
    virtual bool isWorkspaceSupported() const = 0;

    /**
     * Get platform information string
     * @return Human-readable platform and capability information
     */
    virtual std::string getPlatformInfo() const = 0;

    // CONVENIENCE METHODS

    /**
     * Get all windows with workspace information
     * @return Vector of enhanced WindowInfo objects
     */
    virtual std::vector<WindowInfo> getAllWindows() = 0;

    /**
     * Get windows on current workspace only
     * @return Vector of windows on active workspace
     */
    virtual std::vector<WindowInfo> getCurrentWorkspaceWindows() = 0;

    /**
     * Quick search across title and owner fields
     * @param searchTerm Term to search for
     * @return Vector of matched windows
     */
    virtual std::vector<WindowInfo> search(const std::string& searchTerm) = 0;

    /**
     * Get summary of workspace and window information
     * @return JSON string with workspace and window counts
     */
    virtual std::string getSummary() = 0;
};

/**
 * Factory for creating platform-specific enhanced window managers
 */
class EnhancedWindowManagerFactory {
public:
    /**
     * Create enhanced window manager for current platform
     * @return Unique pointer to platform-specific implementation
     */
    static std::unique_ptr<EnhancedWindowManager> create();

    /**
     * Check if current platform is supported
     * @return true if enhanced functionality is available
     */
    static bool isPlatformSupported();

    /**
     * Get list of supported platforms
     * @return Vector of platform names
     */
    static std::vector<std::string> getSupportedPlatforms();
};

} // namespace WindowManager

/**
 * C API for language bindings and external integration
 * Provides C-compatible interface for the enhanced functionality
 */
extern "C" {

typedef struct WMWindowInfo {
    char handle[256];
    char title[512];
    int x, y;
    unsigned int width, height;
    int isVisible;
    unsigned int processId;
    char ownerName[256];

    // Enhanced fields
    char workspaceId[256];
    char workspaceName[256];
    int isOnCurrentWorkspace;
    int state; // WindowState enum value
    int isFocused;
    int isMinimized;
} WMWindowInfo;

typedef struct WMWorkspaceInfo {
    char id[256];
    char name[256];
    int index;
    int isCurrent;
} WMWorkspaceInfo;

// C API function declarations
int wm_initialize();
void wm_cleanup();
int wm_is_workspace_supported();

int wm_enumerate_windows(WMWindowInfo* windows, int max_count);
int wm_enumerate_workspaces(WMWorkspaceInfo* workspaces, int max_count);
int wm_get_current_workspace(WMWorkspaceInfo* workspace);
int wm_search_windows(const char* search_term, WMWindowInfo* windows, int max_count);

const char* wm_get_platform_info();
const char* wm_get_last_error();

} // extern "C"