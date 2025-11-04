#pragma once

#include <string>
#include <chrono>
#include <optional>

namespace WindowManager {

/**
 * Manages workspace switching as part of cross-workspace focus operations
 * Tracks the state and performance of workspace transitions
 */
struct WorkspaceSwitchOperation {
    // ID of workspace before switch
    std::string sourceWorkspaceId;

    // ID of workspace to switch to
    std::string targetWorkspaceId;

    // When switch was initiated
    std::chrono::steady_clock::time_point switchTime;

    // Whether switch completed successfully
    bool completed = false;

    // Platform-specific error code if switch failed (optional)
    std::optional<int> errorCode;

    // When switch completed (optional)
    std::optional<std::chrono::steady_clock::time_point> completionTime;

    // Platform-specific operation data (optional)
    std::string platformData;

    // Default constructor
    WorkspaceSwitchOperation();

    // Parameterized constructor
    WorkspaceSwitchOperation(const std::string& sourceId, const std::string& targetId);

    // Copy constructor and assignment operator
    WorkspaceSwitchOperation(const WorkspaceSwitchOperation& other) = default;
    WorkspaceSwitchOperation& operator=(const WorkspaceSwitchOperation& other) = default;

    // Move constructor and assignment operator
    WorkspaceSwitchOperation(WorkspaceSwitchOperation&& other) noexcept = default;
    WorkspaceSwitchOperation& operator=(WorkspaceSwitchOperation&& other) noexcept = default;

    // Destructor
    ~WorkspaceSwitchOperation() = default;

    // Operation control methods
    void markStarted();
    void markCompleted();
    void markFailed(int platformErrorCode);

    // Validation methods
    bool isValid() const;
    bool isInProgress() const;
    bool isSuccessful() const;
    bool hasFailed() const;

    // Timing methods
    std::chrono::milliseconds getDuration() const;
    bool meetsPerformanceRequirements() const; // Should be < 2 seconds for cross-workspace

    // Utility methods
    std::string toString() const;
    std::string toJson() const;
    std::string getErrorDescription() const;

    // Static factory methods
    static WorkspaceSwitchOperation create(const std::string& source, const std::string& target);

    // Comparison operators
    bool operator==(const WorkspaceSwitchOperation& other) const;
    bool operator!=(const WorkspaceSwitchOperation& other) const;

private:
    // Performance threshold for workspace switching (2 seconds as per SC-002)
    static constexpr std::chrono::milliseconds MAX_SWITCH_DURATION{2000};
};

} // namespace WindowManager