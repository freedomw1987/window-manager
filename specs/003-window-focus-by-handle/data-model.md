# Data Model: Window Focus by Handle

**Branch**: 003-window-focus-by-handle | **Date**: 2025-11-04

## Core Entities

### 1. WindowHandle

**Purpose**: Represents a platform-specific unique identifier for targeting windows

**Fields**:
- `value: string` - Platform-specific window identifier (HWND on Windows, CGWindowID on macOS, Window on X11)
- `platform: string` - Platform identifier ("windows", "macos", "linux")
- `isValid: boolean` - Current validity status of the handle

**Validation Rules**:
- `value` must be non-empty string
- `platform` must be one of the supported platform identifiers
- Handle format validation is platform-specific:
  - Windows: Numeric HWND value
  - macOS: Numeric CGWindowID value
  - Linux: Numeric X11 Window ID

**State Transitions**:
- `valid` → `invalid` when target window is closed or destroyed
- `invalid` → `valid` is not possible (handles are immutable once invalid)

### 2. FocusRequest

**Purpose**: Encapsulates a user request to focus a specific window by handle

**Fields**:
- `targetHandle: WindowHandle` - The window handle to focus
- `timestamp: chrono::steady_clock::time_point` - When the request was initiated
- `requestId: string` - Unique identifier for tracking the request
- `crossWorkspace: boolean` - Whether workspace switching is required
- `sourceWorkspace: string` - Workspace ID where request originated (optional)
- `targetWorkspace: string` - Workspace ID where target window exists (optional)

**Validation Rules**:
- `targetHandle` must be valid WindowHandle object
- `timestamp` must not be in the future
- `requestId` must be unique per session
- If `crossWorkspace` is true, `targetWorkspace` must be provided

**State Transitions**:
- `pending` → `processing` when focus operation begins
- `processing` → `completed` when window is successfully focused
- `processing` → `failed` when focus operation fails
- `processing` → `workspace_switching` when cross-workspace operation required

### 3. FocusOperation

**Purpose**: Tracks the execution state and results of a focus request

**Fields**:
- `request: FocusRequest` - The original focus request
- `status: FocusStatus` - Current status of the operation
- `startTime: chrono::steady_clock::time_point` - Operation start time
- `endTime: chrono::steady_clock::time_point` - Operation completion time (optional)
- `errorMessage: string` - Error description if operation failed (optional)
- `workspaceSwitched: boolean` - Whether workspace switching occurred
- `windowRestored: boolean` - Whether window was restored from minimized state

**Validation Rules**:
- `startTime` must be after or equal to `request.timestamp`
- `endTime` must be after `startTime` when provided
- `errorMessage` required only when `status` is `failed`
- `workspaceSwitched` must be true if cross-workspace operation occurred

### 4. FocusStatus (Enumeration)

**Purpose**: Represents the current state of a focus operation

**Values**:
- `PENDING` - Request received but not yet processed
- `VALIDATING` - Checking handle validity and permissions
- `SWITCHING_WORKSPACE` - Changing to target workspace
- `FOCUSING` - Attempting to focus the target window
- `RESTORING` - Restoring minimized window before focus
- `COMPLETED` - Successfully focused target window
- `FAILED` - Operation failed with error
- `CANCELLED` - Operation was cancelled by user or system

### 5. Enhanced WindowInfo (Extension)

**Purpose**: Extends existing WindowInfo struct with focus-specific capabilities

**New Fields**:
- `lastFocusTime: chrono::steady_clock::time_point` - When window was last focused
- `focusable: boolean` - Whether window can be programmatically focused
- `requiresRestore: boolean` - Whether window needs restoration before focus
- `workspaceSwitchRequired: boolean` - Whether focusing requires workspace change

**Validation Rules**:
- `lastFocusTime` must not be in the future
- `focusable` depends on platform-specific window properties and permissions
- `requiresRestore` true only when `state` is `Minimized`
- `workspaceSwitchRequired` true only when `isOnCurrentWorkspace` is false

### 6. WorkspaceSwitchOperation

**Purpose**: Manages workspace switching as part of cross-workspace focus operations

**Fields**:
- `sourceWorkspaceId: string` - ID of workspace before switch
- `targetWorkspaceId: string` - ID of workspace to switch to
- `switchTime: chrono::steady_clock::time_point` - When switch was initiated
- `completed: boolean` - Whether switch completed successfully
- `errorCode: int` - Platform-specific error code if switch failed (optional)

**Validation Rules**:
- `sourceWorkspaceId` and `targetWorkspaceId` must be valid workspace identifiers
- `sourceWorkspaceId` and `targetWorkspaceId` must be different
- `switchTime` must not be in the future
- `errorCode` required only when `completed` is false

## Relationships

### WindowHandle ↔ WindowInfo
- **Type**: One-to-One
- **Description**: Each WindowHandle corresponds to exactly one WindowInfo object
- **Constraint**: WindowHandle.value must match WindowInfo.handle

### FocusRequest → WindowHandle
- **Type**: Many-to-One
- **Description**: Multiple focus requests can target the same window handle
- **Constraint**: FocusRequest must reference a valid WindowHandle

### FocusOperation ↔ FocusRequest
- **Type**: One-to-One
- **Description**: Each FocusOperation executes exactly one FocusRequest
- **Constraint**: FocusOperation must contain valid FocusRequest

### FocusOperation → WorkspaceSwitchOperation
- **Type**: One-to-Zero-or-One
- **Description**: Cross-workspace focus operations may include workspace switching
- **Constraint**: WorkspaceSwitchOperation exists only when crossWorkspace is true

## Platform-Specific Considerations

### Windows (Win32)
- WindowHandle.value format: String representation of HWND (numeric)
- Virtual Desktop API available Windows 10+ only
- Focus operations may require elevation for some system windows

### macOS (Core Graphics/Cocoa)
- WindowHandle.value format: String representation of CGWindowID (numeric)
- Spaces switching requires private CGS framework
- Accessibility permissions required for focus operations

### Linux (X11)
- WindowHandle.value format: String representation of X11 Window ID (numeric)
- Workspace switching via EWMH (Extended Window Manager Hints)
- Window manager compatibility varies

## Performance Constraints

### Timing Requirements (from Success Criteria)
- Current workspace focus: < 1 second (SC-001)
- Cross-workspace focus: < 2 seconds (SC-002)
- Handle validation: < 0.5 seconds (SC-003)

### Data Volume Limits
- FocusOperation history: Maximum 1000 recent operations per session
- WindowHandle cache: Maximum 10000 handles to prevent memory exhaustion
- Request rate limiting: Maximum 10 focus requests per second per user

## Error Handling

### Common Error Conditions
1. **Invalid Handle**: WindowHandle.value does not correspond to existing window
2. **Permission Denied**: System security prevents focus operation
3. **Workspace Unavailable**: Target workspace cannot be accessed or switched to
4. **Window Destroyed**: Target window was closed during operation
5. **Timeout**: Operation exceeded maximum allowed time

### Error Recovery Strategies
- **Invalid Handle**: Return clear error message with handle validation help
- **Permission Denied**: Provide platform-specific permission resolution steps
- **Workspace Unavailable**: Attempt focus in current workspace if window moved
- **Window Destroyed**: Refresh window list and notify user
- **Timeout**: Cancel operation and report performance issue