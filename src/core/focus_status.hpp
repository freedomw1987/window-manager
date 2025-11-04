#pragma once

#include <string>

namespace WindowManager {

/**
 * Represents the current state of a focus operation
 * Provides granular tracking of focus operation progress
 */
enum class FocusStatus {
    PENDING,              // Request received but not yet processed
    VALIDATING,           // Checking handle validity and permissions
    SWITCHING_WORKSPACE,  // Changing to target workspace
    FOCUSING,             // Attempting to focus the target window
    RESTORING,            // Restoring minimized window before focus
    COMPLETED,            // Successfully focused target window
    FAILED,               // Operation failed with error
    CANCELLED             // Operation was cancelled by user or system
};

/**
 * Utility functions for FocusStatus enumeration
 */
namespace FocusStatusUtils {

/**
 * Convert FocusStatus to string representation
 * @param status The status to convert
 * @return String representation of the status
 */
std::string toString(FocusStatus status);

/**
 * Convert string to FocusStatus enumeration
 * @param statusStr String representation of status
 * @return FocusStatus enumeration value
 * @throws std::invalid_argument if string is not a valid status
 */
FocusStatus fromString(const std::string& statusStr);

/**
 * Check if status represents a terminal state (completed, failed, or cancelled)
 * @param status The status to check
 * @return true if status is terminal, false otherwise
 */
bool isTerminal(FocusStatus status);

/**
 * Check if status represents a successful completion
 * @param status The status to check
 * @return true if status is COMPLETED, false otherwise
 */
bool isSuccessful(FocusStatus status);

/**
 * Check if status represents a failure state
 * @param status The status to check
 * @return true if status is FAILED or CANCELLED, false otherwise
 */
bool isFailure(FocusStatus status);

/**
 * Check if status represents an active/in-progress state
 * @param status The status to check
 * @return true if status is not terminal, false otherwise
 */
bool isActive(FocusStatus status);

/**
 * Get the next expected status in the normal operation flow
 * @param current The current status
 * @return The next expected status, or current if terminal
 */
FocusStatus getNextStatus(FocusStatus current);

/**
 * Get all possible status values as a vector
 * @return Vector containing all FocusStatus values
 */
std::vector<FocusStatus> getAllStatuses();

} // namespace FocusStatusUtils

} // namespace WindowManager