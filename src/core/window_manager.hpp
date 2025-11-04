#pragma once

#include "window.hpp"
#include "workspace.hpp"
#include "enumerator.hpp"
#include "focus_request.hpp"
#include "focus_operation.hpp"
#include <memory>
#include <vector>
#include <chrono>
#include <mutex>
#include <optional>

namespace WindowManager {

// Forward declarations for filtering support (User Story 2)
struct SearchQuery;
struct FilterResult;
class WindowFilter;

// T046: Performance monitoring structure
struct PerformanceMetrics {
    std::chrono::milliseconds windowEnumerationTime{0};
    std::chrono::milliseconds workspaceEnumerationTime{0};
    size_t totalWindowCount = 0;
    size_t totalWorkspaceCount = 0;
    bool windowCacheValid = false;
    bool workspaceCacheValid = false;
    bool meetsWindowPerformanceTarget = false;
    bool meetsWorkspacePerformanceTarget = false;
};

/**
 * Main window manager facade
 * High-level interface combining enumeration and filtering
 * Provides caching and performance optimization
 */
class WindowManager {
public:
    explicit WindowManager(std::unique_ptr<WindowEnumerator> enumerator);
    explicit WindowManager(std::unique_ptr<WindowEnumerator> enumerator,
                          std::unique_ptr<WindowFilter> filter);
    ~WindowManager();

    // Non-copyable, non-moveable (due to mutex)
    WindowManager(const WindowManager&) = delete;
    WindowManager& operator=(const WindowManager&) = delete;
    WindowManager(WindowManager&&) = delete;
    WindowManager& operator=(WindowManager&&) = delete;

    // Primary operations for User Story 1
    std::vector<WindowInfo> getAllWindows();
    bool refreshWindows();

    // Operations for User Story 2 - Keyword Filtering
    FilterResult searchWindows(const std::string& keyword);
    FilterResult searchWindows(const SearchQuery& query);
    FilterResult getEmptyResult(const SearchQuery& query);

    // T042: Operations for User Story 3 - Cross-Workspace Window Management
    std::vector<WorkspaceInfo> getAllWorkspaces();
    std::optional<WorkspaceInfo> getCurrentWorkspace();
    std::vector<WindowInfo> getAllWorkspaceWindows();
    std::vector<WindowInfo> getWindowsOnWorkspace(const std::string& workspaceId);
    std::optional<WindowInfo> getFocusedWindowAcrossWorkspaces();
    FilterResult searchWindowsWithWorkspaces(const SearchQuery& query);

    // NEW: Window Focus Operations (from contracts/focus_api.md)
    bool focusWindowByHandle(const std::string& handle, bool allowWorkspaceSwitch = true);
    bool validateHandle(const std::string& handle);
    bool validateHandleWithTimeout(const std::string& handle, std::chrono::milliseconds timeout);
    std::optional<WindowInfo> getWindowByHandle(const std::string& handle);
    bool focusWindowInCurrentWorkspace(const std::string& handle);
    bool focusWindowAcrossWorkspaces(const std::string& handle);

    // T045: FocusOperation tracking and history
    std::vector<FocusOperation> getFocusHistory() const;
    std::optional<FocusOperation> getLastFocusOperation() const;
    void clearFocusHistory();

    // Performance and state management
    void enableCaching(bool enabled);
    bool isCachingEnabled() const;
    void invalidateCache();

    // Diagnostics and monitoring
    std::chrono::milliseconds getLastUpdateTime() const;
    size_t getTotalWindowCount() const;
    std::string getSystemInfo() const;

    // Success criteria validation
    bool meetsPerformanceRequirements() const; // SC-001, SC-002
    bool supportsRequiredWindowCount() const;  // SC-005

    // T046: Performance monitoring and workspace caching
    std::chrono::milliseconds getLastWorkspaceEnumerationTime() const;
    size_t getWorkspaceCount() const;
    bool meetsWorkspacePerformanceRequirements() const;
    PerformanceMetrics getPerformanceMetrics() const;
    void invalidateWorkspaceCache();
    void refreshAllCaches();

    // Factory method
    static std::unique_ptr<WindowManager> create();

private:
    std::unique_ptr<WindowEnumerator> enumerator_;
    std::unique_ptr<WindowFilter> filter_;
    std::vector<WindowInfo> cachedWindows_;
    std::chrono::steady_clock::time_point lastUpdate_;
    bool cachingEnabled_ = true;
    bool cacheValid_ = false;
    mutable std::mutex cacheMutex_;

    // T046: Workspace caching and performance monitoring
    std::vector<WorkspaceInfo> cachedWorkspaces_;
    std::chrono::steady_clock::time_point lastWorkspaceUpdate_;
    bool workspaceCacheValid_ = false;
    mutable std::mutex workspaceCacheMutex_;
    std::chrono::milliseconds lastWorkspaceEnumerationTime_{0};

    // Performance thresholds from specification
    static constexpr std::chrono::milliseconds MAX_ENUMERATION_TIME{3000}; // 3 seconds (SC-001)
    static constexpr size_t MIN_SUPPORTED_WINDOWS = 50; // SC-005 requirement
    static constexpr size_t MAX_CACHE_SIZE = 10000; // Limit cache to prevent excessive memory usage
    static constexpr std::chrono::seconds CACHE_VALIDITY_DURATION{5}; // Cache valid for 5 seconds

    // T046: Workspace cache performance thresholds
    static constexpr std::chrono::milliseconds WORKSPACE_ENUMERATION_WARNING_THRESHOLD{1000}; // 1 second
    static constexpr std::chrono::seconds WORKSPACE_CACHE_VALIDITY_DURATION{10}; // Workspaces change less frequently

    // T036, T039: Timeout and rate limiting thresholds
    static constexpr std::chrono::milliseconds DEFAULT_VALIDATION_TIMEOUT{500}; // 0.5 seconds for validation
    static constexpr size_t MAX_FOCUS_REQUESTS_PER_SECOND = 10; // Rate limiting for focus requests
    static constexpr std::chrono::seconds RATE_LIMIT_WINDOW{1}; // Rate limiting time window

    // T039: Rate limiting state
    mutable std::mutex rateLimitMutex_;
    std::vector<std::chrono::steady_clock::time_point> focusRequestTimes_;

    // T045: FocusOperation tracking and history
    mutable std::mutex focusHistoryMutex_;
    std::vector<FocusOperation> focusHistory_;
    static constexpr size_t MAX_FOCUS_HISTORY_SIZE = 1000; // Limit history to prevent memory growth

    // Cache management
    void updateCache();
    bool isCacheValid() const;

    // T046: Workspace cache management
    void updateWorkspaceCache();
    bool isWorkspaceCacheValid() const;

    // T039: Rate limiting helpers
    bool checkRateLimit();
    void recordFocusRequest();

    // T045: Focus operation tracking helpers
    void addToFocusHistory(const FocusOperation& operation);
};

} // namespace WindowManager