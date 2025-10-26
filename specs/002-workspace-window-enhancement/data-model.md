# Data Model: Workspace Window Enhancement

**Feature**: Workspace Window Enhancement
**Created**: 2025-10-26
**Purpose**: Define enhanced data structures for workspace-aware window management

## Core Entities

### Enhanced WindowInfo Structure

```cpp
namespace WindowManager {

enum class WindowState {
    Normal,     // Standard visible window
    Minimized,  // Hidden/iconified window
    Focused,    // Currently active window
    Hidden      // Hidden but not minimized (e.g., on different workspace)
};

/**
 * Enhanced window information with workspace and state support
 * Extends existing WindowInfo with workspace identification and state detection
 */
struct WindowInfo {
    // Existing fields (preserved for backward compatibility)
    std::string handle;                    // Universal window identifier
    std::string title;                     // Window display title
    int x, y;                             // Position coordinates
    unsigned int width, height;           // Window dimensions
    bool isVisible;                       // Basic visibility flag
    unsigned int processId;               // Process identifier
    std::string ownerName;                // Application/process name

    // NEW: Workspace information
    std::string workspaceId;              // Platform-specific workspace identifier
    std::string workspaceName;            // Human-readable workspace name
    bool isOnCurrentWorkspace;            // Quick check for current workspace

    // NEW: Enhanced state information
    WindowState state;                    // Comprehensive window state
    bool isFocused;                       // Currently focused window
    bool isMinimized;                     // Explicitly minimized state

    // Constructors
    WindowInfo();
    WindowInfo(const std::string& handle, const std::string& title,
               int x, int y, unsigned int width, unsigned int height,
               bool visible, unsigned int pid, const std::string& owner,
               const std::string& workspaceId = "", const std::string& workspaceName = "",
               bool onCurrentWorkspace = true, WindowState state = WindowState::Normal);

    // Validation methods
    bool isValid() const;
    bool hasValidDimensions() const;
    bool hasValidPosition() const;
    bool hasWorkspaceInfo() const;        // NEW: Workspace information availability

    // Display and formatting methods
    std::string toString() const;         // Enhanced with workspace info
    std::string toJson() const;           // Enhanced JSON with new fields
    std::string toShortString() const;    // NEW: Compact display format

    // Comparison operators (updated for new fields)
    bool operator==(const WindowInfo& other) const;
    bool operator!=(const WindowInfo& other) const;
    bool operator<(const WindowInfo& other) const;
};

} // namespace WindowManager
```

### WorkspaceInfo Structure

```cpp
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

    WorkspaceInfo();
    WorkspaceInfo(const std::string& id, const std::string& name,
                  int index, bool current = false);

    // Validation and utility methods
    bool isValid() const;
    std::string toString() const;
    std::string toJson() const;

    // Comparison operators
    bool operator==(const WorkspaceInfo& other) const;
    bool operator<(const WorkspaceInfo& other) const;
};

} // namespace WindowManager
```

### Enhanced Search Query

```cpp
namespace WindowManager {

enum class SearchField {
    Title,      // Search in window title
    Owner,      // Search in application/owner name
    Both        // Search in both title and owner (default)
};

/**
 * Enhanced search query supporting multiple fields
 * Backward compatible with existing single-field searches
 */
struct SearchQuery {
    std::string query;                    // Search term
    SearchField field;                    // Which field(s) to search
    bool caseSensitive;                   // Case sensitivity flag
    bool useRegex;                        // Regular expression mode
    std::string workspaceFilter;          // Filter by workspace ID (empty = all)

    SearchQuery(const std::string& query, SearchField field = SearchField::Both,
                bool caseSensitive = false, bool useRegex = false);

    // Validation and utility methods
    bool isValid() const;
    std::string toString() const;

    // Match testing
    bool matches(const WindowInfo& window) const;
    bool matchesTitle(const std::string& title) const;
    bool matchesOwner(const std::string& owner) const;
};

} // namespace WindowManager
```

### Filter Result Enhancement

```cpp
namespace WindowManager {

/**
 * Enhanced filter result with workspace grouping and statistics
 */
struct FilterResult {
    std::vector<WindowInfo> windows;                    // Filtered windows
    std::map<std::string, std::vector<WindowInfo>> byWorkspace; // Grouped by workspace

    // Statistics
    size_t totalCount;                                  // Total windows found
    size_t currentWorkspaceCount;                       // Windows on current workspace
    std::map<std::string, size_t> workspaceCounts;     // Count per workspace

    // Query information
    SearchQuery originalQuery;                          // Query that produced this result
    std::chrono::milliseconds searchTime;              // Time taken for search

    FilterResult();
    FilterResult(const std::vector<WindowInfo>& windows, const SearchQuery& query);

    // Result processing
    void groupByWorkspace();
    void calculateStatistics();
    void sortResults(bool byWorkspace = false, bool byTitle = true);

    // Output formatting
    std::string toString(bool includeWorkspaceInfo = true) const;
    std::string toJson(bool includeStatistics = true) const;
    std::string toSummary() const;                      // Brief summary

    // Filtering and manipulation
    FilterResult filterByWorkspace(const std::string& workspaceId) const;
    FilterResult filterByState(WindowState state) const;
};

} // namespace WindowManager
```

## Entity Relationships

### WindowInfo → WorkspaceInfo
- **Relationship**: Many-to-One
- **Key**: WindowInfo.workspaceId → WorkspaceInfo.id
- **Description**: Each window belongs to exactly one workspace (or spans all workspaces)

### SearchQuery → FilterResult
- **Relationship**: One-to-One
- **Description**: Each search query produces one filter result
- **Caching**: Results may be cached based on query parameters

### FilterResult → WindowInfo
- **Relationship**: One-to-Many
- **Description**: Each filter result contains multiple windows
- **Grouping**: Windows grouped by workspace for enhanced display

## Validation Rules

### WindowInfo Validation
```cpp
bool WindowInfo::hasWorkspaceInfo() const {
    return !workspaceId.empty() || !workspaceName.empty();
}

bool WindowInfo::isValid() const {
    return !handle.empty() &&
           processId > 0 &&
           hasValidDimensions() &&
           (workspaceId.empty() || workspaceId.length() > 0); // Workspace ID if present must be non-empty
}
```

### SearchQuery Validation
```cpp
bool SearchQuery::isValid() const {
    return !query.empty() &&
           query.length() <= 1000 &&  // Reasonable length limit
           (field == SearchField::Title || field == SearchField::Owner || field == SearchField::Both);
}
```

### WorkspaceInfo Validation
```cpp
bool WorkspaceInfo::isValid() const {
    return !id.empty() &&
           index >= 0 &&
           !name.empty();
}
```

## State Transitions

### WindowState Transitions
```
Normal ↔ Minimized   (User minimizes/restores window)
Normal ↔ Focused     (Window gains/loses focus)
Normal ↔ Hidden      (Window moves to different workspace)
Minimized → Focused  (Restoring minimized window gives it focus)
```

### Workspace State Changes
```
WorkspaceInfo.isCurrent: false → true   (User switches to workspace)
WindowInfo.isOnCurrentWorkspace: true ↔ false   (Based on current workspace)
```

## Platform-Specific Considerations

### Windows Implementation
```cpp
// Platform-specific workspace ID format
// workspaceId = GUID string (e.g., "{12345678-1234-5678-9ABC-DEF012345678}")
// workspaceName = "Desktop 1", "Desktop 2", etc.
```

### macOS Implementation
```cpp
// Platform-specific workspace ID format
// workspaceId = CGSSpaceID as string (e.g., "1", "2", "3")
// workspaceName = "Desktop 1", "Desktop 2", etc. (from user preferences if available)
```

### Linux Implementation
```cpp
// Platform-specific workspace ID format
// workspaceId = Desktop index as string (e.g., "0", "1", "2")
// workspaceName = Desktop name from _NET_DESKTOP_NAMES
```

## Backward Compatibility

### JSON Output Format
```json
{
  "handle": "string",
  "title": "string",
  "x": 0, "y": 0, "width": 100, "height": 100,
  "isVisible": true,
  "processId": 1234,
  "ownerName": "application.exe",

  // NEW FIELDS (added to existing structure)
  "workspaceId": "workspace-id",
  "workspaceName": "Workspace Name",
  "isOnCurrentWorkspace": true,
  "state": "Normal", // "Normal", "Minimized", "Focused", "Hidden"
  "isFocused": false,
  "isMinimized": false
}
```

### Field Migration Strategy
- All existing fields preserved exactly
- New fields added with sensible defaults
- Old code continues to work without modification
- New fields ignored by legacy consumers
- Enhanced functionality available to updated consumers