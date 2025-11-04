#pragma once

#include <exception>
#include <string>
#include <chrono>
#include <map>
#include <vector>

namespace WindowManager {

/**
 * Base exception class for window management operations
 */
class WindowManagerException : public std::exception {
public:
    explicit WindowManagerException(const std::string& message);
    const char* what() const noexcept override;

protected:
    std::string message_;
};

/**
 * Exception thrown when a platform is not supported
 */
class PlatformNotSupportedException : public WindowManagerException {
public:
    explicit PlatformNotSupportedException(const std::string& platform);
};

/**
 * Exception thrown when permissions are denied
 * (e.g., macOS accessibility permissions, Windows UAC)
 */
class PermissionDeniedException : public WindowManagerException {
public:
    explicit PermissionDeniedException(const std::string& details);
};

/**
 * Exception thrown when window enumeration fails
 */
class WindowEnumerationException : public WindowManagerException {
public:
    explicit WindowEnumerationException(const std::string& details);
};

/**
 * Exception thrown when a window operation fails
 */
class WindowOperationException : public WindowManagerException {
public:
    explicit WindowOperationException(const std::string& operation, const std::string& details);
};

/**
 * Exception thrown when platform-specific API calls fail
 */
class PlatformApiException : public WindowManagerException {
public:
    explicit PlatformApiException(const std::string& apiName, int errorCode, const std::string& details = "");

    int getErrorCode() const noexcept { return errorCode_; }
    const std::string& getApiName() const noexcept { return apiName_; }

private:
    std::string apiName_;
    int errorCode_;
};

/**
 * Exception thrown when filtering operations fail
 */
class FilterException : public WindowManagerException {
public:
    explicit FilterException(const std::string& details);
};

/**
 * Exception thrown when configuration is invalid
 */
class ConfigurationException : public WindowManagerException {
public:
    explicit ConfigurationException(const std::string& parameter, const std::string& issue);
};

// T047: Enhanced error handling and graceful degradation

/**
 * Exception thrown when workspace operations fail but can be degraded gracefully
 */
class WorkspaceException : public WindowManagerException {
public:
    explicit WorkspaceException(const std::string& details, bool canDegrade = true);
    bool canDegrade() const noexcept { return canDegrade_; }

private:
    bool canDegrade_;
};

/**
 * Exception thrown when window handle is invalid or malformed
 */
class InvalidHandleException : public WindowManagerException {
public:
    explicit InvalidHandleException(const std::string& handle, const std::string& reason = "");
    const std::string& getHandle() const noexcept { return handle_; }

private:
    std::string handle_;
};

/**
 * Exception thrown when workspace switching fails
 */
class WorkspaceSwitchException : public WorkspaceException {
public:
    explicit WorkspaceSwitchException(const std::string& sourceWorkspace,
                                    const std::string& targetWorkspace,
                                    const std::string& reason);

    const std::string& getSourceWorkspace() const noexcept { return sourceWorkspace_; }
    const std::string& getTargetWorkspace() const noexcept { return targetWorkspace_; }

private:
    std::string sourceWorkspace_;
    std::string targetWorkspace_;
};

/**
 * Exception thrown when focus operation fails
 */
class FocusException : public WindowManagerException {
public:
    explicit FocusException(const std::string& handle, const std::string& reason);
    const std::string& getHandle() const noexcept { return handle_; }

private:
    std::string handle_;
};

/**
 * Exception thrown when performance targets are not met but operation can continue
 */
class PerformanceWarningException : public WindowManagerException {
public:
    explicit PerformanceWarningException(const std::string& operation,
                                       std::chrono::milliseconds actualTime,
                                       std::chrono::milliseconds targetTime);

    std::chrono::milliseconds getActualTime() const noexcept { return actualTime_; }
    std::chrono::milliseconds getTargetTime() const noexcept { return targetTime_; }

private:
    std::chrono::milliseconds actualTime_;
    std::chrono::milliseconds targetTime_;
};

/**
 * Utility class for graceful degradation and error recovery
 */
class ErrorRecovery {
public:
    enum class FallbackStrategy {
        ReturnEmpty,        // Return empty results
        UseCache,          // Use cached results if available
        UseLimitedData,    // Return partial/limited data
        RetryWithSimpler   // Retry with simpler approach
    };

    enum class ErrorSeverity {
        Warning,    // Operation succeeded with warnings
        Recoverable, // Operation failed but recovered
        Critical    // Operation failed, no recovery possible
    };

    // Factory methods for common error scenarios
    static WindowManagerException createPlatformFallback(const std::string& feature, const std::string& platform);
    static WindowManagerException createPermissionFallback(const std::string& operation);
    static WindowManagerException createPerformanceFallback(const std::string& operation, std::chrono::milliseconds time);

    // Error severity assessment
    static ErrorSeverity assessSeverity(const std::exception& e);
    static bool canRecover(const std::exception& e);
    static FallbackStrategy recommendStrategy(const std::exception& e);

    // Graceful degradation helpers
    static std::string getErrorSummary(const std::exception& e);
    static std::string getSuggestion(const std::exception& e);
};

/**
 * Comprehensive error context for detailed error reporting
 */
struct ErrorContext {
    std::string operation;
    std::string platform;
    std::chrono::steady_clock::time_point timestamp;
    std::string component;
    std::map<std::string, std::string> additionalInfo;

    ErrorContext(const std::string& op, const std::string& comp = "");
    std::string toString() const;
};

/**
 * Error aggregation for collecting multiple non-critical errors
 */
class ErrorAggregator {
public:
    void addWarning(const std::string& message, const std::string& component = "");
    void addError(const std::exception& e, const std::string& component = "");

    bool hasWarnings() const { return !warnings_.empty(); }
    bool hasErrors() const { return !errors_.empty(); }
    size_t getWarningCount() const { return warnings_.size(); }
    size_t getErrorCount() const { return errors_.size(); }

    std::string getWarningsSummary() const;
    std::string getErrorsSummary() const;
    std::string getFullSummary() const;

    void clear();

private:
    std::vector<std::pair<std::string, std::string>> warnings_; // message, component
    std::vector<std::pair<std::string, std::string>> errors_;   // message, component
};

} // namespace WindowManager