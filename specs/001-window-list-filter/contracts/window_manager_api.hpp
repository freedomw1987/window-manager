/**
 * Window Manager API Contract
 * Defines the interface contracts for cross-platform window management
 */

#pragma once

#include <vector>
#include <string>
#include <memory>
#include <chrono>
#include <optional>

namespace WindowManager {

// Forward declarations
struct WindowInfo;
struct SearchQuery;
struct FilterResult;
class WindowEnumerator;

/**
 * Core window information structure
 * Represents a single window with all necessary attributes
 */
struct WindowInfo {
    // Universal window identifier (platform-specific type erased)
    std::string handle;

    // Window display properties
    std::string title;
    int x, y;                    // Position coordinates
    unsigned int width, height; // Dimensions
    bool isVisible;

    // Process information
    unsigned int processId;
    std::string ownerName;

    // Constructors
    WindowInfo() = default;
    WindowInfo(const std::string& handle, const std::string& title,
               int x, int y, unsigned int width, unsigned int height,
               bool visible, unsigned int pid, const std::string& owner);

    // Validation
    bool isValid() const;

    // String representation for display
    std::string toString() const;
    std::string toJson() const;
};

/**
 * Search query parameters
 * Encapsulates user filtering criteria
 */
struct SearchQuery {
    enum class MatchMode {
        CONTAINS,       // Substring search (default)
        STARTS_WITH,    // Prefix matching
        EXACT,          // Exact match
        REGEX           // Regular expression (optional)
    };

    std::string keyword;
    bool caseSensitive = false;
    MatchMode matchMode = MatchMode::CONTAINS;
    std::chrono::steady_clock::time_point timestamp;

    // Constructors
    SearchQuery() = default;
    explicit SearchQuery(const std::string& keyword,
                        bool caseSensitive = false,
                        MatchMode mode = MatchMode::CONTAINS);

    // Query operations
    bool matches(const WindowInfo& window) const;
    bool isEmpty() const;
};

/**
 * Filter operation result
 * Contains filtered windows and performance metrics
 */
struct FilterResult {
    std::vector<WindowInfo> windows;
    size_t totalCount;
    size_t filteredCount;
    std::chrono::milliseconds searchTime;
    SearchQuery query;

    // Constructors
    FilterResult() = default;
    FilterResult(std::vector<WindowInfo> windows, size_t total,
                const SearchQuery& query, std::chrono::milliseconds time);

    // Result analysis
    double filterRatio() const;
    bool meetsPerformanceTarget() const; // <1 second requirement
    std::string getSummary() const;
};

/**
 * Window enumeration interface
 * Abstract base for platform-specific implementations
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

    // Performance and diagnostics
    virtual std::chrono::milliseconds getLastEnumerationTime() const = 0;
    virtual size_t getWindowCount() const = 0;
    virtual std::string getPlatformInfo() const = 0;

    // Factory method
    static std::unique_ptr<WindowEnumerator> create();
};

/**
 * Window filter interface
 * Handles search and filtering operations
 */
class WindowFilter {
public:
    virtual ~WindowFilter() = default;

    // Core filtering operations
    virtual FilterResult filter(const std::vector<WindowInfo>& windows,
                               const SearchQuery& query) = 0;

    // Convenience methods
    FilterResult filterByKeyword(const std::vector<WindowInfo>& windows,
                                const std::string& keyword);
    FilterResult filterVisible(const std::vector<WindowInfo>& windows);

    // Performance optimization
    virtual void setCaching(bool enabled) = 0;
    virtual void clearCache() = 0;

    // Factory method
    static std::unique_ptr<WindowFilter> create();
};

/**
 * Main window manager facade
 * High-level interface combining enumeration and filtering
 */
class WindowManager {
public:
    explicit WindowManager(std::unique_ptr<WindowEnumerator> enumerator,
                          std::unique_ptr<WindowFilter> filter);
    ~WindowManager() = default;

    // Non-copyable, moveable
    WindowManager(const WindowManager&) = delete;
    WindowManager& operator=(const WindowManager&) = delete;
    WindowManager(WindowManager&&) = default;
    WindowManager& operator=(WindowManager&&) = default;

    // Primary operations matching functional requirements

    // FR-001: System MUST enumerate all currently open windows
    std::vector<WindowInfo> getAllWindows();

    // FR-005: System MUST provide keyword-based filtering
    FilterResult searchWindows(const std::string& keyword);
    FilterResult searchWindows(const SearchQuery& query);

    // FR-009: System MUST handle no matches gracefully
    FilterResult getEmptyResult(const SearchQuery& query);

    // Performance and state management
    bool refreshWindows();
    void enableCaching(bool enabled);

    // Diagnostics and monitoring
    std::chrono::milliseconds getLastUpdateTime() const;
    size_t getTotalWindowCount() const;
    std::string getSystemInfo() const;

    // Success criteria validation
    bool meetsPerformanceRequirements() const; // SC-001, SC-002
    bool supportsRequiredWindowCount() const;  // SC-005

    // Factory method
    static std::unique_ptr<WindowManager> create();

private:
    std::unique_ptr<WindowEnumerator> enumerator_;
    std::unique_ptr<WindowFilter> filter_;
    std::vector<WindowInfo> cachedWindows_;
    std::chrono::steady_clock::time_point lastUpdate_;
    bool cachingEnabled_ = true;
};

/**
 * Command-line interface contract
 * Defines the CLI interaction patterns
 */
class CommandLineInterface {
public:
    virtual ~CommandLineInterface() = default;

    // User interaction modes matching user stories

    // User Story 1: Basic window listing
    virtual void displayAllWindows(const std::vector<WindowInfo>& windows) = 0;

    // User Story 2: Keyword filtering
    virtual std::string getSearchKeyword() = 0;
    virtual void displayFilteredResults(const FilterResult& result) = 0;

    // User Story 3: Interactive filtering
    virtual bool runInteractiveMode(WindowManager& manager) = 0;

    // Display and formatting
    virtual void displayHelp() = 0;
    virtual void displayError(const std::string& message) = 0;
    virtual void displayPerformanceStats(const FilterResult& result) = 0;

    // Configuration
    virtual void setOutputFormat(const std::string& format) = 0; // text, json
    virtual void setVerbose(bool verbose) = 0;

    // Factory method
    static std::unique_ptr<CommandLineInterface> create();
};

/**
 * Application controller
 * Orchestrates the entire application flow
 */
class Application {
public:
    Application();
    ~Application() = default;

    // Main entry points matching acceptance scenarios
    int runOneShotMode(const std::vector<std::string>& args);
    int runInteractiveMode();
    int runJsonMode(const std::string& keyword = "");

    // Configuration
    void setVerbose(bool verbose);
    void setOutputFormat(const std::string& format);
    void setTimeout(std::chrono::seconds timeout);

    // Error handling
    void handlePlatformError(const std::string& error);
    void displayUsage();

private:
    std::unique_ptr<WindowManager> windowManager_;
    std::unique_ptr<CommandLineInterface> cli_;
    bool verbose_ = false;
    std::string outputFormat_ = "text";
    std::chrono::seconds timeout_ = std::chrono::seconds(30);
};

/**
 * Platform-specific error types
 * For handling platform API failures gracefully
 */
class WindowManagerException : public std::exception {
public:
    explicit WindowManagerException(const std::string& message);
    const char* what() const noexcept override;

private:
    std::string message_;
};

class PlatformNotSupportedException : public WindowManagerException {
public:
    explicit PlatformNotSupportedException(const std::string& platform);
};

class PermissionDeniedException : public WindowManagerException {
public:
    explicit PermissionDeniedException(const std::string& details);
};

class WindowEnumerationException : public WindowManagerException {
public:
    explicit WindowEnumerationException(const std::string& details);
};

} // namespace WindowManager