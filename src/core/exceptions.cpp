#include "exceptions.hpp"
#include <sstream>

namespace WindowManager {

// WindowManagerException implementation
WindowManagerException::WindowManagerException(const std::string& message)
    : message_(message) {
}

const char* WindowManagerException::what() const noexcept {
    return message_.c_str();
}

// PlatformNotSupportedException implementation
PlatformNotSupportedException::PlatformNotSupportedException(const std::string& platform)
    : WindowManagerException("Platform not supported: " + platform +
                             ". This application supports Windows, macOS, and Linux only.") {
}

// PermissionDeniedException implementation
PermissionDeniedException::PermissionDeniedException(const std::string& details)
    : WindowManagerException("Permission denied: " + details) {
}

// WindowEnumerationException implementation
WindowEnumerationException::WindowEnumerationException(const std::string& details)
    : WindowManagerException("Window enumeration failed: " + details) {
}

// WindowOperationException implementation
WindowOperationException::WindowOperationException(const std::string& operation, const std::string& details)
    : WindowManagerException("Window operation '" + operation + "' failed: " + details) {
}

// PlatformApiException implementation
PlatformApiException::PlatformApiException(const std::string& apiName, int errorCode, const std::string& details)
    : WindowManagerException([&]() {
        std::ostringstream oss;
        oss << "Platform API call failed: " << apiName << " (error code: " << errorCode << ")";
        if (!details.empty()) {
            oss << " - " << details;
        }
        return oss.str();
    }())
    , apiName_(apiName)
    , errorCode_(errorCode) {
}

// FilterException implementation
FilterException::FilterException(const std::string& details)
    : WindowManagerException("Filter operation failed: " + details) {
}

// ConfigurationException implementation
ConfigurationException::ConfigurationException(const std::string& parameter, const std::string& issue)
    : WindowManagerException("Configuration error for parameter '" + parameter + "': " + issue) {
}

// T047: Enhanced error handling and graceful degradation implementation

// WorkspaceException implementation
WorkspaceException::WorkspaceException(const std::string& details, bool canDegrade)
    : WindowManagerException("Workspace operation failed: " + details +
                             (canDegrade ? " (graceful fallback available)" : " (critical failure)"))
    , canDegrade_(canDegrade) {
}

// PerformanceWarningException implementation
PerformanceWarningException::PerformanceWarningException(const std::string& operation,
                                                       std::chrono::milliseconds actualTime,
                                                       std::chrono::milliseconds targetTime)
    : WindowManagerException([&]() {
        std::ostringstream oss;
        oss << "Performance warning for '" << operation << "': took " << actualTime.count()
            << "ms (target: " << targetTime.count() << "ms)";
        return oss.str();
    }())
    , actualTime_(actualTime)
    , targetTime_(targetTime) {
}

// ErrorRecovery implementation
WindowManagerException ErrorRecovery::createPlatformFallback(const std::string& feature, const std::string& platform) {
    return WindowManagerException(
        "Feature '" + feature + "' not available on " + platform +
        ". Falling back to basic functionality."
    );
}

WindowManagerException ErrorRecovery::createPermissionFallback(const std::string& operation) {
    return WindowManagerException(
        "Permission denied for '" + operation + "'. " +
        "Continuing with limited functionality. " +
        "Grant necessary permissions for full features."
    );
}

WindowManagerException ErrorRecovery::createPerformanceFallback(const std::string& operation, std::chrono::milliseconds time) {
    std::ostringstream oss;
    oss << "Performance degradation detected for '" << operation << "' (" << time.count() << "ms). ";
    oss << "Consider reducing data set size or enabling caching.";
    return WindowManagerException(oss.str());
}

ErrorRecovery::ErrorSeverity ErrorRecovery::assessSeverity(const std::exception& e) {
    // Check exception type to determine severity
    if (dynamic_cast<const PerformanceWarningException*>(&e)) {
        return ErrorSeverity::Warning;
    }
    if (dynamic_cast<const WorkspaceException*>(&e)) {
        const auto* we = dynamic_cast<const WorkspaceException*>(&e);
        return we->canDegrade() ? ErrorSeverity::Recoverable : ErrorSeverity::Critical;
    }
    if (dynamic_cast<const PlatformNotSupportedException*>(&e)) {
        return ErrorSeverity::Critical;
    }
    if (dynamic_cast<const PermissionDeniedException*>(&e)) {
        return ErrorSeverity::Recoverable;
    }
    if (dynamic_cast<const FilterException*>(&e)) {
        return ErrorSeverity::Recoverable;
    }
    // T035: Focus-specific exception severity assessment
    if (dynamic_cast<const InvalidHandleException*>(&e)) {
        return ErrorSeverity::Recoverable; // User can provide a different handle
    }
    if (dynamic_cast<const FocusException*>(&e)) {
        return ErrorSeverity::Recoverable; // Focus operation can be retried
    }
    if (dynamic_cast<const WorkspaceSwitchException*>(&e)) {
        return ErrorSeverity::Recoverable; // Can try focus without workspace switching
    }

    // Default to recoverable for unknown exceptions
    return ErrorSeverity::Recoverable;
}

bool ErrorRecovery::canRecover(const std::exception& e) {
    ErrorSeverity severity = assessSeverity(e);
    return severity != ErrorSeverity::Critical;
}

ErrorRecovery::FallbackStrategy ErrorRecovery::recommendStrategy(const std::exception& e) {
    if (dynamic_cast<const PermissionDeniedException*>(&e)) {
        return FallbackStrategy::UseLimitedData;
    }
    if (dynamic_cast<const WorkspaceException*>(&e)) {
        return FallbackStrategy::RetryWithSimpler;
    }
    if (dynamic_cast<const PerformanceWarningException*>(&e)) {
        return FallbackStrategy::UseCache;
    }
    if (dynamic_cast<const WindowEnumerationException*>(&e)) {
        return FallbackStrategy::UseCache;
    }
    // T035: Focus-specific fallback strategies
    if (dynamic_cast<const InvalidHandleException*>(&e)) {
        return FallbackStrategy::ReturnEmpty; // User needs to provide valid handle
    }
    if (dynamic_cast<const FocusException*>(&e)) {
        return FallbackStrategy::RetryWithSimpler; // Retry without workspace switching
    }
    if (dynamic_cast<const WorkspaceSwitchException*>(&e)) {
        return FallbackStrategy::RetryWithSimpler; // Retry focus without workspace switch
    }

    return FallbackStrategy::ReturnEmpty;
}

std::string ErrorRecovery::getErrorSummary(const std::exception& e) {
    std::ostringstream oss;
    oss << "Error: " << e.what();

    ErrorSeverity severity = assessSeverity(e);
    switch (severity) {
        case ErrorSeverity::Warning:
            oss << " [WARNING]";
            break;
        case ErrorSeverity::Recoverable:
            oss << " [RECOVERABLE]";
            break;
        case ErrorSeverity::Critical:
            oss << " [CRITICAL]";
            break;
    }

    return oss.str();
}

std::string ErrorRecovery::getSuggestion(const std::exception& e) {
    if (dynamic_cast<const PermissionDeniedException*>(&e)) {
        return "Grant the required permissions and try again.";
    }
    if (dynamic_cast<const WorkspaceException*>(&e)) {
        return "Check workspace/virtual desktop support on your system.";
    }
    if (dynamic_cast<const PerformanceWarningException*>(&e)) {
        return "Consider enabling caching or reducing the data set size.";
    }
    if (dynamic_cast<const PlatformNotSupportedException*>(&e)) {
        return "Use a supported platform (Windows, macOS, or Linux).";
    }
    // T035: Focus-specific error suggestions
    if (dynamic_cast<const InvalidHandleException*>(&e)) {
        return "Use 'window-manager list --show-handles' to see available window handles, or verify the handle format is correct.";
    }
    if (dynamic_cast<const FocusException*>(&e)) {
        return "Check if the window still exists and is accessible. Try running without workspace switching if the error persists.";
    }
    if (dynamic_cast<const WorkspaceSwitchException*>(&e)) {
        return "Try focusing the window without workspace switching using --no-workspace-switch option, or check if the target workspace exists.";
    }

    return "Please report this issue with your system details.";
}

// T035: Detailed error message generation for focus-specific failures

// InvalidHandleException implementation
InvalidHandleException::InvalidHandleException(const std::string& handle, const std::string& reason)
    : WindowManagerException([&]() {
        std::ostringstream oss;
        oss << "Invalid window handle: '" << handle << "'";
        if (!reason.empty()) {
            oss << " - " << reason;
        }
        return oss.str();
    }())
    , handle_(handle) {
}

// WorkspaceSwitchException implementation
WorkspaceSwitchException::WorkspaceSwitchException(const std::string& sourceWorkspace,
                                                 const std::string& targetWorkspace,
                                                 const std::string& reason)
    : WorkspaceException([&]() {
        std::ostringstream oss;
        oss << "Failed to switch workspace from '" << sourceWorkspace
            << "' to '" << targetWorkspace << "'";
        if (!reason.empty()) {
            oss << " - " << reason;
        }
        return oss.str();
    }(), false) // workspace switching failures are not degradable
    , sourceWorkspace_(sourceWorkspace)
    , targetWorkspace_(targetWorkspace) {
}

// FocusException implementation
FocusException::FocusException(const std::string& handle, const std::string& reason)
    : WindowManagerException([&]() {
        std::ostringstream oss;
        oss << "Failed to focus window with handle '" << handle << "'";
        if (!reason.empty()) {
            oss << " - " << reason;
        }
        return oss.str();
    }())
    , handle_(handle) {
}

// ErrorContext implementation
ErrorContext::ErrorContext(const std::string& op, const std::string& comp)
    : operation(op)
    , timestamp(std::chrono::steady_clock::now())
    , component(comp) {

    // Detect platform automatically
#ifdef _WIN32
    platform = "Windows";
#elif __APPLE__
    platform = "macOS";
#elif __linux__
    platform = "Linux";
#else
    platform = "Unknown";
#endif
}

std::string ErrorContext::toString() const {
    std::ostringstream oss;
    oss << "Operation: " << operation << "\n";
    oss << "Component: " << component << "\n";
    oss << "Platform: " << platform << "\n";

    // Convert steady_clock timestamp to system_clock for display
    auto now_steady = std::chrono::steady_clock::now();
    auto now_system = std::chrono::system_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now_steady - timestamp);
    auto timestamp_system = now_system - elapsed;

    auto time_t = std::chrono::system_clock::to_time_t(timestamp_system);
    oss << "Timestamp: " << std::ctime(&time_t);

    if (!additionalInfo.empty()) {
        oss << "Additional Info:\n";
        for (const auto& pair : additionalInfo) {
            oss << "  " << pair.first << ": " << pair.second << "\n";
        }
    }

    return oss.str();
}

// ErrorAggregator implementation
void ErrorAggregator::addWarning(const std::string& message, const std::string& component) {
    warnings_.emplace_back(message, component);
}

void ErrorAggregator::addError(const std::exception& e, const std::string& component) {
    errors_.emplace_back(e.what(), component);
}

std::string ErrorAggregator::getWarningsSummary() const {
    if (warnings_.empty()) {
        return "No warnings";
    }

    std::ostringstream oss;
    oss << warnings_.size() << " warning(s):\n";
    for (size_t i = 0; i < warnings_.size(); ++i) {
        oss << "  " << (i + 1) << ". " << warnings_[i].first;
        if (!warnings_[i].second.empty()) {
            oss << " [" << warnings_[i].second << "]";
        }
        oss << "\n";
    }
    return oss.str();
}

std::string ErrorAggregator::getErrorsSummary() const {
    if (errors_.empty()) {
        return "No errors";
    }

    std::ostringstream oss;
    oss << errors_.size() << " error(s):\n";
    for (size_t i = 0; i < errors_.size(); ++i) {
        oss << "  " << (i + 1) << ". " << errors_[i].first;
        if (!errors_[i].second.empty()) {
            oss << " [" << errors_[i].second << "]";
        }
        oss << "\n";
    }
    return oss.str();
}

std::string ErrorAggregator::getFullSummary() const {
    std::ostringstream oss;

    if (!warnings_.empty()) {
        oss << getWarningsSummary();
    }

    if (!errors_.empty()) {
        if (!warnings_.empty()) {
            oss << "\n";
        }
        oss << getErrorsSummary();
    }

    if (warnings_.empty() && errors_.empty()) {
        oss << "No warnings or errors";
    }

    return oss.str();
}

void ErrorAggregator::clear() {
    warnings_.clear();
    errors_.clear();
}

} // namespace WindowManager