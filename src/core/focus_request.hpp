#pragma once

#include <string>
#include <chrono>

namespace WindowManager {

/**
 * Encapsulates a user request to focus a specific window by handle
 * Tracks request details and workspace switching requirements
 */
struct FocusRequest {
    // Target window handle to focus
    std::string targetHandle;

    // When the request was initiated
    std::chrono::steady_clock::time_point timestamp;

    // Unique identifier for tracking the request
    std::string requestId;

    // Whether workspace switching is required
    bool crossWorkspace = false;

    // Workspace ID where request originated (optional)
    std::string sourceWorkspace;

    // Workspace ID where target window exists (optional)
    std::string targetWorkspace;

    // Default constructor
    FocusRequest();

    // Parameterized constructor
    FocusRequest(const std::string& handle, const std::string& id = "");

    // Full constructor with workspace information
    FocusRequest(const std::string& handle, const std::string& id,
                 bool requiresWorkspaceSwitch, const std::string& sourceWS = "",
                 const std::string& targetWS = "");

    // Copy constructor and assignment operator
    FocusRequest(const FocusRequest& other) = default;
    FocusRequest& operator=(const FocusRequest& other) = default;

    // Move constructor and assignment operator
    FocusRequest(FocusRequest&& other) noexcept = default;
    FocusRequest& operator=(FocusRequest&& other) noexcept = default;

    // Destructor
    ~FocusRequest() = default;

    // Validation methods
    bool isValid() const;
    bool hasValidHandle() const;
    bool requiresWorkspaceSwitch() const;

    // Utility methods
    std::string toString() const;
    std::string toJson() const;

    // Generate unique request ID
    static std::string generateRequestId();

    // Comparison operators
    bool operator==(const FocusRequest& other) const;
    bool operator!=(const FocusRequest& other) const;
};

} // namespace WindowManager