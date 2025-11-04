#pragma once

#include "focus_request.hpp"
#include "focus_status.hpp"
#include <string>
#include <chrono>
#include <optional>

namespace WindowManager {

/**
 * Tracks the execution state and results of a focus request
 * Provides detailed information about the focus operation lifecycle
 */
struct FocusOperation {
    // The original focus request
    FocusRequest request;

    // Current status of the operation
    FocusStatus status;

    // Operation start time
    std::chrono::steady_clock::time_point startTime;

    // Operation completion time (optional)
    std::optional<std::chrono::steady_clock::time_point> endTime;

    // Error description if operation failed (optional)
    std::string errorMessage;

    // Whether workspace switching occurred
    bool workspaceSwitched = false;

    // Whether window was restored from minimized state
    bool windowRestored = false;

    // Platform-specific error code (optional)
    int platformErrorCode = 0;

    // Default constructor
    FocusOperation();

    // Constructor from focus request
    explicit FocusOperation(const FocusRequest& req);

    // Constructor with initial status
    FocusOperation(const FocusRequest& req, FocusStatus initialStatus);

    // Copy constructor and assignment operator
    FocusOperation(const FocusOperation& other) = default;
    FocusOperation& operator=(const FocusOperation& other) = default;

    // Move constructor and assignment operator
    FocusOperation(FocusOperation&& other) noexcept = default;
    FocusOperation& operator=(FocusOperation&& other) noexcept = default;

    // Destructor
    ~FocusOperation() = default;

    // Status update methods
    void setStatus(FocusStatus newStatus);
    void complete();
    void fail(const std::string& error, int errorCode = 0);
    void markWorkspaceSwitched();
    void markWindowRestored();

    // Timing methods
    std::chrono::milliseconds getDuration() const;
    bool isComplete() const;
    bool isSuccessful() const;
    bool hasFailed() const;

    // Validation methods
    bool isValid() const;
    bool meetsPerformanceRequirements() const;

    // Output methods
    std::string toString() const;
    std::string toJson() const;
    std::string getStatusString() const;

    // Comparison operators
    bool operator==(const FocusOperation& other) const;
    bool operator!=(const FocusOperation& other) const;
};

} // namespace WindowManager