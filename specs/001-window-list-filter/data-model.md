# Data Model: Window List and Filter Program

**Created**: 2025-10-26
**Context**: Core data structures and relationships for cross-platform window management

## Core Entities

### WindowInfo
Represents an open application window with all necessary attributes for display and filtering.

**Fields**:
- `handle`: Platform-specific window identifier (HWND on Windows, Window on X11, CGWindowID on macOS)
- `title`: Window title/caption text (UTF-8 string)
- `x`: X coordinate of window position (pixels, relative to primary display)
- `y`: Y coordinate of window position (pixels, relative to primary display)
- `width`: Window width in pixels
- `height`: Window height in pixels
- `isVisible`: Boolean indicating if window is currently visible/mapped
- `processId`: Process ID of the application owning this window
- `ownerName`: Name of the application/process owning the window

**Validation Rules**:
- `title` must not be null (empty strings allowed for windows without titles)
- `width` and `height` must be > 0 for valid windows
- `x` and `y` can be negative (off-screen or multi-monitor scenarios)
- `handle` must be unique within a single enumeration session
- `processId` must be > 0 for valid processes

**State Transitions**:
- Window can transition between visible/invisible states
- Position and size can change during window lifetime
- Title can change dynamically
- Handle remains constant for window lifetime

### SearchQuery
Represents user input for filtering windows with matching criteria.

**Fields**:
- `keyword`: Search term entered by user (UTF-8 string)
- `caseSensitive`: Boolean flag for case-sensitive matching (default: false)
- `matchMode`: Enumeration for matching behavior (CONTAINS, STARTS_WITH, EXACT)
- `timestamp`: When the search was initiated (for performance tracking)

**Validation Rules**:
- `keyword` can be empty (shows all windows)
- `keyword` length should be reasonable (<1000 characters)
- `matchMode` must be valid enumeration value
- `timestamp` used for performance measurement

**State Transitions**:
- Query is created when user enters/modifies search text
- Query is applied to current window list
- Results are filtered based on query criteria

### FilterResult
Represents the output of applying a search query to window list.

**Fields**:
- `windows`: Vector of WindowInfo objects matching the search criteria
- `totalCount`: Total number of windows before filtering
- `filteredCount`: Number of windows after filtering
- `searchTime`: Time taken to perform the search (milliseconds)
- `query`: Copy of the SearchQuery that generated this result

**Validation Rules**:
- `filteredCount` must be <= `totalCount`
- `filteredCount` must equal `windows.size()`
- `searchTime` should be < 1000ms per specification requirement
- `windows` should be sorted for consistent presentation

## Platform-Specific Extensions

### Win32WindowInfo (Windows)
**Additional Fields**:
- `className`: Windows class name from GetClassName()
- `dwStyle`: Window style flags from GetWindowLong()
- `dwExStyle`: Extended window style flags
- `threadId`: Thread ID owning the window

### X11WindowInfo (Linux)
**Additional Fields**:
- `windowClass`: X11 window class from WM_CLASS property
- `desktop`: Desktop/workspace number (EWMH _NET_WM_DESKTOP)
- `windowType`: Window type hint (EWMH _NET_WM_WINDOW_TYPE)
- `windowState`: Window state flags (EWMH _NET_WM_STATE)

### CocoaWindowInfo (macOS)
**Additional Fields**:
- `windowLayer`: CGWindowLayer for stacking order
- `ownerPID`: Process ID from Core Graphics
- `memoryUsage`: Window memory usage if available
- `isOnScreen`: Boolean from kCGWindowIsOnscreen

## Data Relationships

### WindowEnumerator → WindowInfo
- One-to-many relationship: Single enumerator produces multiple windows
- Temporal relationship: Enumeration represents snapshot at specific time
- Platform relationship: Different enumerators for different platforms

### SearchQuery → FilterResult
- One-to-one relationship: Each query produces one result
- Temporal relationship: Result represents query application at specific time
- Performance relationship: Query complexity affects result generation time

### WindowInfo → FilterResult
- Many-to-many relationship: Windows can appear in multiple search results
- Filter relationship: WindowInfo objects filtered by SearchQuery criteria

## Memory Management Strategy

### WindowInfo Lifecycle
1. **Creation**: During enumeration from platform APIs
2. **Storage**: In vector containers for fast iteration
3. **Filtering**: Copied to filtered results (lightweight objects)
4. **Cleanup**: Automatic destruction when containers go out of scope

### Search Performance Optimization
- **String Optimization**: Use string_view for filtering operations to avoid copies
- **Result Caching**: Cache filtered results for repeated identical queries
- **Incremental Updates**: For real-time filtering, update results incrementally

### Platform Handle Management
- **RAII Principles**: Platform-specific resources wrapped in smart pointers
- **Resource Cleanup**: Automatic cleanup of platform resources (X11 Display*, etc.)
- **Exception Safety**: Strong exception safety guarantee for all operations

## Serialization Format

### JSON Output (Optional Feature)
```json
{
  "windows": [
    {
      "handle": "platform-specific-id",
      "title": "Window Title",
      "position": {"x": 100, "y": 200},
      "size": {"width": 800, "height": 600},
      "visible": true,
      "processId": 1234,
      "owner": "Application Name"
    }
  ],
  "metadata": {
    "totalCount": 15,
    "filteredCount": 3,
    "searchTime": 45,
    "query": "search term",
    "timestamp": "2025-10-26T10:30:00Z"
  }
}
```

### Text Output Format
```
Windows (3 of 15):
[1] Application Name - Window Title
    Position: (100, 200)  Size: 800x600  PID: 1234

[2] Another App - Another Window
    Position: (300, 400)  Size: 1024x768  PID: 5678

Search completed in 45ms
```

## Error Handling Data

### WindowEnumerationError
**Fields**:
- `errorCode`: Platform-specific error code
- `message`: Human-readable error description
- `platform`: Which platform API failed
- `timestamp`: When error occurred

**Common Error Scenarios**:
- Permission denied (especially macOS accessibility)
- Display not available (Linux X11)
- System API failure (Windows)
- Memory allocation failures

This data model provides a robust foundation for the window management application while maintaining clear separation between platform-specific implementations and cross-platform business logic.