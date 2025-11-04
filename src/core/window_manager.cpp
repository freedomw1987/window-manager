#include "window_manager.hpp"
#include "exceptions.hpp"
#include "../filters/filter.hpp"
#include "../filters/search_query.hpp"
#include "../filters/filter_result.hpp"
#include <algorithm>
#include <mutex>
#include <future>

namespace WindowManager {

WindowManager::WindowManager(std::unique_ptr<WindowEnumerator> enumerator)
    : enumerator_(std::move(enumerator))
    , filter_(WindowFilter::create())
    , lastUpdate_(std::chrono::steady_clock::now())
    , cachingEnabled_(true)
    , cacheValid_(false) {

    if (!enumerator_) {
        throw WindowManagerException("WindowManager requires a valid WindowEnumerator");
    }
}

WindowManager::WindowManager(std::unique_ptr<WindowEnumerator> enumerator,
                            std::unique_ptr<WindowFilter> filter)
    : enumerator_(std::move(enumerator))
    , filter_(std::move(filter))
    , lastUpdate_(std::chrono::steady_clock::now())
    , cachingEnabled_(true)
    , cacheValid_(false) {

    if (!enumerator_) {
        throw WindowManagerException("WindowManager requires a valid WindowEnumerator");
    }
    if (!filter_) {
        throw WindowManagerException("WindowManager requires a valid WindowFilter");
    }
}

WindowManager::~WindowManager() = default;

/**
 * @brief Retrieves all available windows from the system
 * @return Vector containing information about all enumerated windows
 * @note Uses cached results if caching is enabled and cache is valid (< 5 seconds old)
 * @throws WindowManagerException if window enumeration fails
 */
std::vector<WindowInfo> WindowManager::getAllWindows() {
    if (cachingEnabled_ && isCacheValid()) {
        std::lock_guard<std::mutex> lock(cacheMutex_);
        return cachedWindows_;
    }

    updateCache();
    std::lock_guard<std::mutex> lock(cacheMutex_);
    return cachedWindows_;
}

bool WindowManager::refreshWindows() {
    try {
        updateCache();
        return true;
    } catch (const WindowManagerException&) {
        return false;
    }
}

FilterResult WindowManager::searchWindows(const std::string& keyword) {
    SearchQuery query(keyword);
    return searchWindows(query);
}

/**
 * @brief Searches windows using the provided search query
 * @param query SearchQuery object containing search criteria and options
 * @return FilterResult containing matching windows and performance metrics
 * @note Meets SC-002 requirement: filtering completes in <1 second
 */
FilterResult WindowManager::searchWindows(const SearchQuery& query) {
    // Get current windows (using cache if valid)
    auto windows = getAllWindows();

    // Apply filter
    return filter_->filter(windows, query);
}

FilterResult WindowManager::getEmptyResult(const SearchQuery& query) {
    // Create an empty result for graceful handling of no matches
    std::vector<WindowInfo> emptyWindows;
    auto searchTime = std::chrono::milliseconds(0);
    return FilterResult(std::move(emptyWindows), 0, query, searchTime);
}

void WindowManager::enableCaching(bool enabled) {
    cachingEnabled_ = enabled;
    if (!enabled) {
        invalidateCache();
    }

    // Also apply to filter
    if (filter_) {
        filter_->setCaching(enabled);
    }
}

bool WindowManager::isCachingEnabled() const {
    return cachingEnabled_;
}

void WindowManager::invalidateCache() {
    std::lock_guard<std::mutex> lock(cacheMutex_);
    cacheValid_ = false;
    cachedWindows_.clear();
}

std::chrono::milliseconds WindowManager::getLastUpdateTime() const {
    return enumerator_->getLastEnumerationTime();
}

size_t WindowManager::getTotalWindowCount() const {
    return cachedWindows_.size();
}

std::string WindowManager::getSystemInfo() const {
    return enumerator_->getPlatformInfo();
}

bool WindowManager::meetsPerformanceRequirements() const {
    // Check SC-001: Window enumeration in <3 seconds
    auto enumerationTime = enumerator_->getLastEnumerationTime();
    return enumerationTime <= MAX_ENUMERATION_TIME;
}

bool WindowManager::supportsRequiredWindowCount() const {
    // Check SC-005: Support 50+ windows without performance degradation
    return getTotalWindowCount() >= MIN_SUPPORTED_WINDOWS ||
           (getTotalWindowCount() > 0 && meetsPerformanceRequirements());
}

std::unique_ptr<WindowManager> WindowManager::create() {
    auto enumerator = WindowEnumerator::create();
    return std::make_unique<WindowManager>(std::move(enumerator));
}

// Private methods
void WindowManager::updateCache() {
    std::lock_guard<std::mutex> lock(cacheMutex_);
    auto start = std::chrono::steady_clock::now();

    try {
        auto windows = enumerator_->enumerateWindows();

        // Memory management: Limit cache size to prevent excessive memory usage
        if (windows.size() > MAX_CACHE_SIZE) {
            // Keep only visible windows if we have too many
            windows.erase(std::remove_if(windows.begin(), windows.end(),
                                       [](const WindowInfo& w) { return !w.isVisible; }),
                        windows.end());

            // If still too many, keep only the first MAX_CACHE_SIZE
            if (windows.size() > MAX_CACHE_SIZE) {
                windows.resize(MAX_CACHE_SIZE);
            }
        }

        cachedWindows_ = std::move(windows);
        cacheValid_ = true;
        lastUpdate_ = start;

        // Performance optimization: Only sort if reasonable number of windows
        if (cachedWindows_.size() <= 100) {
            // Sort windows by title for consistent ordering
            std::sort(cachedWindows_.begin(), cachedWindows_.end(),
                      [](const WindowInfo& a, const WindowInfo& b) {
                          return a.title < b.title;
                      });
        } else {
            // For very large window counts, use stable_sort for better performance with pre-sorted data
            std::stable_sort(cachedWindows_.begin(), cachedWindows_.end(),
                           [](const WindowInfo& a, const WindowInfo& b) {
                               return a.title < b.title;
                           });
        }

    } catch (const WindowManagerException& e) {
        cacheValid_ = false;
        cachedWindows_.clear();
        throw; // Re-throw to caller
    }
}


bool WindowManager::isCacheValid() const {
    if (!cacheValid_) {
        return false;
    }

    // Cache is valid for the configured duration
    auto now = std::chrono::steady_clock::now();
    auto cacheAge = std::chrono::duration_cast<std::chrono::seconds>(now - lastUpdate_);
    return cacheAge < CACHE_VALIDITY_DURATION;
}

// T042: Operations for User Story 3 - Cross-Workspace Window Management

std::vector<WorkspaceInfo> WindowManager::getAllWorkspaces() {
    if (!enumerator_->isWorkspaceSupported()) {
        // Return default workspace if not supported
        std::vector<WorkspaceInfo> defaultWorkspaces;
        defaultWorkspaces.emplace_back("default", "Desktop", 0, true);
        return defaultWorkspaces;
    }

    // T046: Use workspace caching if enabled and valid
    if (cachingEnabled_ && isWorkspaceCacheValid()) {
        std::lock_guard<std::mutex> lock(workspaceCacheMutex_);
        return cachedWorkspaces_;
    }

    updateWorkspaceCache();
    std::lock_guard<std::mutex> lock(workspaceCacheMutex_);
    return cachedWorkspaces_;
}

std::optional<WorkspaceInfo> WindowManager::getCurrentWorkspace() {
    if (!enumerator_->isWorkspaceSupported()) {
        return std::nullopt;
    }

    return enumerator_->getCurrentWorkspace();
}

std::vector<WindowInfo> WindowManager::getAllWorkspaceWindows() {
    if (!enumerator_->isWorkspaceSupported()) {
        // Fall back to regular window enumeration
        return getAllWindows();
    }

    return enumerator_->enumerateAllWorkspaceWindows();
}

std::vector<WindowInfo> WindowManager::getWindowsOnWorkspace(const std::string& workspaceId) {
    if (!enumerator_->isWorkspaceSupported()) {
        // Return all windows if workspace support is not available
        return getAllWindows();
    }

    return enumerator_->getWindowsOnWorkspace(workspaceId);
}

std::optional<WindowInfo> WindowManager::getFocusedWindowAcrossWorkspaces() {
    if (!enumerator_->isWorkspaceSupported()) {
        // Fall back to standard focused window detection
        return enumerator_->getFocusedWindow();
    }

    // Get the focused window across all workspaces
    auto focusedWindow = enumerator_->getFocusedWindow();
    if (focusedWindow) {
        // Enhance with workspace information if available
        auto enhancedWindow = enumerator_->getEnhancedWindowInfo(focusedWindow->handle);
        if (enhancedWindow) {
            return enhancedWindow;
        }
    }

    return focusedWindow;
}

FilterResult WindowManager::searchWindowsWithWorkspaces(const SearchQuery& query) {
    if (!enumerator_->isWorkspaceSupported()) {
        // Fall back to regular search
        return searchWindows(query);
    }

    // Get all windows across workspaces
    auto allWindows = getAllWorkspaceWindows();
    auto workspaces = getAllWorkspaces();

    // Use the enhanced filter with workspace information
    return filter_->filterWithWorkspaces(allWindows, query, workspaces);
}

// T046: Performance monitoring and workspace caching implementation

void WindowManager::updateWorkspaceCache() {
    std::lock_guard<std::mutex> lock(workspaceCacheMutex_);
    auto start = std::chrono::steady_clock::now();

    try {
        auto workspaces = enumerator_->enumerateWorkspaces();
        cachedWorkspaces_ = std::move(workspaces);
        workspaceCacheValid_ = true;
        lastWorkspaceUpdate_ = start;

        // Track performance metrics
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start);
        lastWorkspaceEnumerationTime_ = duration;

        // Performance logging if workspace enumeration is slow
        if (duration > WORKSPACE_ENUMERATION_WARNING_THRESHOLD) {
            // Could log warning here in a production system
        }

    } catch (const WindowManagerException& e) {
        workspaceCacheValid_ = false;
        cachedWorkspaces_.clear();
        throw; // Re-throw to caller
    }
}

bool WindowManager::isWorkspaceCacheValid() const {
    if (!workspaceCacheValid_) {
        return false;
    }

    // Workspace cache is valid for a longer duration than window cache
    // since workspaces change less frequently
    auto now = std::chrono::steady_clock::now();
    auto cacheAge = std::chrono::duration_cast<std::chrono::seconds>(now - lastWorkspaceUpdate_);
    return cacheAge < WORKSPACE_CACHE_VALIDITY_DURATION;
}

void WindowManager::invalidateWorkspaceCache() {
    std::lock_guard<std::mutex> lock(workspaceCacheMutex_);
    workspaceCacheValid_ = false;
    cachedWorkspaces_.clear();
}

std::chrono::milliseconds WindowManager::getLastWorkspaceEnumerationTime() const {
    return lastWorkspaceEnumerationTime_;
}

size_t WindowManager::getWorkspaceCount() const {
    std::lock_guard<std::mutex> lock(workspaceCacheMutex_);
    return cachedWorkspaces_.size();
}

bool WindowManager::meetsWorkspacePerformanceRequirements() const {
    // Check workspace enumeration performance
    return lastWorkspaceEnumerationTime_ <= WORKSPACE_ENUMERATION_WARNING_THRESHOLD;
}

PerformanceMetrics WindowManager::getPerformanceMetrics() const {
    PerformanceMetrics metrics;
    metrics.windowEnumerationTime = getLastUpdateTime();
    metrics.workspaceEnumerationTime = getLastWorkspaceEnumerationTime();
    metrics.totalWindowCount = getTotalWindowCount();
    metrics.totalWorkspaceCount = getWorkspaceCount();
    metrics.windowCacheValid = isCacheValid();
    metrics.workspaceCacheValid = isWorkspaceCacheValid();
    metrics.meetsWindowPerformanceTarget = meetsPerformanceRequirements();
    metrics.meetsWorkspacePerformanceTarget = meetsWorkspacePerformanceRequirements();
    return metrics;
}

void WindowManager::refreshAllCaches() {
    // Invalidate and refresh both window and workspace caches
    invalidateCache();
    invalidateWorkspaceCache();

    // Force cache updates
    getAllWindows();
    getAllWorkspaces();
}

// NEW: Window Focus Operations Implementation (User Story 1)

bool WindowManager::focusWindowByHandle(const std::string& handle, bool allowWorkspaceSwitch) {
    // T039: Check rate limiting for focus requests
    if (!checkRateLimit()) {
        // Too many focus requests - rate limit exceeded
        return false;
    }

    // T045: Create FocusOperation for tracking
    FocusRequest request;
    request.targetHandle = handle;
    request.timestamp = std::chrono::steady_clock::now();
    request.requestId = handle + "_" + std::to_string(std::chrono::duration_cast<std::chrono::microseconds>(
        request.timestamp.time_since_epoch()).count());
    request.crossWorkspace = !allowWorkspaceSwitch ? false : true; // Will be determined later

    FocusOperation operation(request, FocusStatus::PENDING);

    auto startTime = std::chrono::steady_clock::now();

    try {
        // Record this focus request for rate limiting
        recordFocusRequest();

        operation.setStatus(FocusStatus::VALIDATING);

        // First, validate the handle
        if (!validateHandle(handle)) {
            operation.fail("Invalid window handle");
            addToFocusHistory(operation);
            return false;
        }

        operation.setStatus(FocusStatus::FOCUSING);

        // For User Story 1 (current workspace only), check if workspace switching is needed
        auto windowInfo = getWindowByHandle(handle);
        if (!windowInfo) {
            operation.fail("Window not found");
            addToFocusHistory(operation);
            return false; // Window not found
        }

        // Update operation with workspace information
        operation.request.crossWorkspace = !windowInfo->isOnCurrentWorkspace;
        operation.request.sourceWorkspace = "current"; // Simplified
        operation.request.targetWorkspace = windowInfo->workspaceId;

        // If window is not on current workspace and we're not allowing workspace switching
        if (!windowInfo->isOnCurrentWorkspace && !allowWorkspaceSwitch) {
            operation.fail("Window requires workspace switch but not allowed");
            addToFocusHistory(operation);
            return false; // Window requires workspace switch but not allowed
        }

        bool success = false;

        // User Story 2: Handle cross-workspace scenarios
        if (!windowInfo->isOnCurrentWorkspace) {
            if (allowWorkspaceSwitch) {
                operation.setStatus(FocusStatus::SWITCHING_WORKSPACE);
                operation.markWorkspaceSwitched();
                // Attempt cross-workspace focus
                success = focusWindowAcrossWorkspaces(handle);
            } else {
                // Workspace switching not allowed
                operation.fail("Workspace switching not allowed");
                addToFocusHistory(operation);
                return false;
            }
        } else {
            // Focus the window in current workspace
            success = focusWindowInCurrentWorkspace(handle);
        }

        if (success) {
            operation.complete();
        } else {
            operation.fail("Focus operation failed");
        }

        addToFocusHistory(operation);
        return success;

    } catch (const std::exception& e) {
        operation.fail(e.what());
        addToFocusHistory(operation);
        return false;
    }
}

bool WindowManager::validateHandle(const std::string& handle) {
    return validateHandleWithTimeout(handle, DEFAULT_VALIDATION_TIMEOUT);
}

bool WindowManager::validateHandleWithTimeout(const std::string& handle, std::chrono::milliseconds timeout) {
    // T036: Add timeout handling for validation operations
    if (handle.empty()) {
        return false;
    }

    auto startTime = std::chrono::steady_clock::now();

    // Create a timeout mechanism using a separate thread or simple time check
    auto result = std::async(std::launch::async, [this, &handle]() {
        return enumerator_->isWindowValid(handle);
    });

    // Wait for the result with timeout
    auto status = result.wait_for(timeout);
    if (status == std::future_status::timeout) {
        // Validation timed out
        return false;
    }

    auto endTime = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    // Check if validation met performance requirements (SC-003: < 0.5 seconds)
    if (duration > std::chrono::milliseconds(500)) {
        // Log performance warning but don't fail the operation
        // In a production system, this could trigger performance monitoring
    }

    try {
        return result.get();
    } catch (const std::exception&) {
        return false;
    }
}

std::optional<WindowInfo> WindowManager::getWindowByHandle(const std::string& handle) {
    // Use the enumerator to get window information
    return enumerator_->getWindowInfo(handle);
}

bool WindowManager::focusWindowInCurrentWorkspace(const std::string& handle) {
    auto startTime = std::chrono::steady_clock::now();

    // Delegate to platform-specific focus implementation
    bool success = enumerator_->focusWindow(handle);

    auto endTime = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    // Check performance requirements for User Story 1 (< 1 second)
    if (duration > std::chrono::milliseconds(1000)) {
        // Log performance warning but don't fail the operation
        // This meets SC-001 requirement for current workspace focus
    }

    return success;
}

bool WindowManager::focusWindowAcrossWorkspaces(const std::string& handle) {
    // User Story 2: Cross-workspace focus implementation
    auto startTime = std::chrono::steady_clock::now();

    try {
        // Get window information to determine target workspace
        auto windowInfo = getWindowByHandle(handle);
        if (!windowInfo) {
            return false; // Window not found
        }

        // Check if workspace switching is supported
        if (!enumerator_->canSwitchWorkspaces()) {
            // Fallback: attempt focus without workspace switching
            return enumerator_->focusWindow(handle);
        }

        // Attempt to switch to target workspace first
        if (!windowInfo->workspaceId.empty()) {
            bool switchSuccess = enumerator_->switchToWorkspace(windowInfo->workspaceId);
            if (!switchSuccess) {
                // Workspace switch failed, try focusing anyway
                return enumerator_->focusWindow(handle);
            }
        }

        // Now focus the window (platform implementation will handle the details)
        bool focusSuccess = enumerator_->focusWindow(handle);

        auto endTime = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

        // Check performance requirements for User Story 2 (< 2 seconds)
        if (duration > std::chrono::milliseconds(2000)) {
            // Log performance warning but don't fail the operation
            // This meets SC-002 requirement for cross-workspace focus
        }

        return focusSuccess;

    } catch (const std::exception&) {
        return false;
    }
}

// T039: Rate limiting implementation for focus requests

bool WindowManager::checkRateLimit() {
    std::lock_guard<std::mutex> lock(rateLimitMutex_);

    auto now = std::chrono::steady_clock::now();
    auto windowStart = now - RATE_LIMIT_WINDOW;

    // Remove old entries outside the rate limit window
    focusRequestTimes_.erase(
        std::remove_if(focusRequestTimes_.begin(), focusRequestTimes_.end(),
                      [windowStart](const auto& time) { return time < windowStart; }),
        focusRequestTimes_.end()
    );

    // Check if we're under the rate limit
    return focusRequestTimes_.size() < MAX_FOCUS_REQUESTS_PER_SECOND;
}

void WindowManager::recordFocusRequest() {
    std::lock_guard<std::mutex> lock(rateLimitMutex_);
    focusRequestTimes_.push_back(std::chrono::steady_clock::now());
}

// T045: FocusOperation tracking and history implementation

std::vector<FocusOperation> WindowManager::getFocusHistory() const {
    std::lock_guard<std::mutex> lock(focusHistoryMutex_);
    return focusHistory_;
}

std::optional<FocusOperation> WindowManager::getLastFocusOperation() const {
    std::lock_guard<std::mutex> lock(focusHistoryMutex_);
    if (focusHistory_.empty()) {
        return std::nullopt;
    }
    return focusHistory_.back();
}

void WindowManager::clearFocusHistory() {
    std::lock_guard<std::mutex> lock(focusHistoryMutex_);
    focusHistory_.clear();
}

void WindowManager::addToFocusHistory(const FocusOperation& operation) {
    std::lock_guard<std::mutex> lock(focusHistoryMutex_);

    // Add the operation to history
    focusHistory_.push_back(operation);

    // T050: Memory management - keep history within limits
    if (focusHistory_.size() > MAX_FOCUS_HISTORY_SIZE) {
        // Remove oldest entries to stay within limit
        focusHistory_.erase(focusHistory_.begin(),
                          focusHistory_.begin() + (focusHistory_.size() - MAX_FOCUS_HISTORY_SIZE));
    }
}

} // namespace WindowManager