#pragma once

#include <string>
#include <vector>

namespace WindowManager {

/**
 * Workspace/Virtual Desktop information
 * Represents a single workspace with its properties and state
 */
struct WorkspaceInfo {
    std::string id;                       // Platform-specific unique identifier
    std::string name;                     // Human-readable name
    int index;                           // Zero-based workspace index
    bool isCurrent;                      // Whether this is the active workspace
    std::vector<std::string> windowHandles; // Handles of windows on this workspace

    // Platform-specific data (stored as variant or JSON string)
    std::string platformData;            // Platform-specific workspace metadata

    // Default constructor
    WorkspaceInfo();

    // Parameterized constructor
    WorkspaceInfo(const std::string& id, const std::string& name,
                  int index, bool current = false);

    // Copy constructor and assignment operator
    WorkspaceInfo(const WorkspaceInfo& other) = default;
    WorkspaceInfo& operator=(const WorkspaceInfo& other) = default;

    // Move constructor and assignment operator
    WorkspaceInfo(WorkspaceInfo&& other) noexcept = default;
    WorkspaceInfo& operator=(WorkspaceInfo&& other) noexcept = default;

    // Destructor
    ~WorkspaceInfo() = default;

    // Validation and utility methods
    bool isValid() const;
    std::string toString() const;
    std::string toJson() const;

    // Comparison operators
    bool operator==(const WorkspaceInfo& other) const;
    bool operator!=(const WorkspaceInfo& other) const;
    bool operator<(const WorkspaceInfo& other) const;
};

} // namespace WindowManager