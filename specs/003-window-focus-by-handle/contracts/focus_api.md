# Focus API Contract

**Branch**: 003-window-focus-by-handle | **Date**: 2025-11-04

## API Overview

The Window Focus API extends the existing CLI interface with handle-based window focusing capabilities. This follows the established pattern of command-line arguments for window operations.

## Command Interface Contracts

### 1. Focus Window Command

**Command**: `window-manager focus <handle>`

**Description**: Focus a window by its platform-specific handle, with automatic workspace switching if needed.

**Parameters**:
- `handle` (required): Platform-specific window handle as string
  - Windows: HWND numeric value
  - macOS: CGWindowID numeric value
  - Linux: X11 Window ID numeric value

**Options**:
- `--no-workspace-switch`: Prevent automatic workspace switching (fail if window not in current workspace)
- `--timeout <seconds>`: Override default operation timeout (default: 5 seconds)
- `--verbose`: Show detailed operation progress
- `--json`: Output results in JSON format

**Exit Codes**:
- `0`: Success - Window focused successfully
- `1`: Invalid handle - Handle format invalid or window not found
- `2`: Permission denied - Insufficient permissions to focus window
- `3`: Workspace error - Cannot switch to target workspace
- `4`: Timeout - Operation exceeded maximum time limit
- `5`: General error - Other system or application error

**Examples**:
```bash
# Focus window by handle (auto workspace switch)
window-manager focus 12345

# Focus window in current workspace only
window-manager focus 12345 --no-workspace-switch

# Focus with custom timeout and verbose output
window-manager focus 12345 --timeout 10 --verbose

# Get JSON output for programmatic use
window-manager focus 12345 --json
```

### 2. Validate Handle Command

**Command**: `window-manager validate-handle <handle>`

**Description**: Validate a window handle without performing focus operation.

**Parameters**:
- `handle` (required): Window handle to validate

**Options**:
- `--json`: Output validation results in JSON format

**Exit Codes**:
- `0`: Valid handle - Window exists and can be focused
- `1`: Invalid handle - Handle malformed or window not found
- `2`: Permission denied - Window exists but cannot be accessed
- `3`: Not focusable - Window exists but cannot be focused

**Examples**:
```bash
# Validate handle
window-manager validate-handle 12345

# Get detailed validation results
window-manager validate-handle 12345 --json
```

### 3. List Windows with Handles Command

**Command**: `window-manager list --show-handles`

**Description**: Extend existing list command to show window handles for copy/paste workflow.

**Enhanced Options**:
- `--show-handles`: Include window handles in output
- `--handles-only`: Show only handles and titles (compact format)

**Examples**:
```bash
# List all windows with handles visible
window-manager list --show-handles

# Compact format showing handles and titles only
window-manager list --handles-only
```

## Output Format Contracts

### 1. Success Output (Text Format)

```text
✓ Window focused successfully
  Handle: 12345
  Title: "Application Window"
  Workspace: Desktop 2 (switched from Desktop 1)
  Time: 0.8 seconds
```

### 2. Success Output (JSON Format)

```json
{
  "status": "success",
  "operation": {
    "type": "focus_window",
    "handle": "12345",
    "duration_ms": 800,
    "workspace_switched": true,
    "window_restored": false
  },
  "window": {
    "handle": "12345",
    "title": "Application Window",
    "workspace": {
      "id": "2",
      "name": "Desktop 2"
    }
  },
  "source_workspace": {
    "id": "1",
    "name": "Desktop 1"
  }
}
```

### 3. Error Output (Text Format)

```text
✗ Failed to focus window
  Handle: 99999
  Error: Window not found
  Suggestion: Use 'window-manager list --show-handles' to see available windows
```

### 4. Error Output (JSON Format)

```json
{
  "status": "error",
  "error": {
    "code": "WINDOW_NOT_FOUND",
    "message": "Window not found",
    "handle": "99999",
    "suggestion": "Use 'window-manager list --show-handles' to see available windows"
  },
  "operation": {
    "type": "focus_window",
    "duration_ms": 150
  }
}
```

## Integration with Existing API

### 1. WindowManager Class Extensions

**New Method**: `bool focusWindowByHandle(const std::string& handle, bool allowWorkspaceSwitch = true)`

**Parameters**:
- `handle`: Platform-specific window handle
- `allowWorkspaceSwitch`: Whether to switch workspaces if needed

**Returns**: `true` if focus succeeded, `false` otherwise

**Throws**:
- `InvalidHandleException` for malformed handles
- `PermissionDeniedException` for access restrictions
- `WorkspaceException` for workspace switching failures

### 2. CLI Class Extensions

**New Methods**:
- `void displayFocusSuccess(const FocusOperation& operation)`
- `void displayFocusError(const std::string& handle, const std::string& error, const std::string& suggestion = "")`
- `void displayFocusProgress(const FocusOperation& operation)` (for verbose mode)

### 3. Platform Enumerator Extensions

**Enhanced Method**: Extend existing `focusWindow()` to support workspace switching

**New Method**: `bool switchToWorkspace(const std::string& workspaceId)`

## Error Code Mapping

| Exit Code | Error Type | Description | Recovery Action |
|-----------|------------|-------------|-----------------|
| 0 | Success | Window focused successfully | None |
| 1 | INVALID_HANDLE | Handle format invalid or window not found | Check handle format, list current windows |
| 2 | PERMISSION_DENIED | Insufficient permissions to focus window | Check system permissions, run as administrator if needed |
| 3 | WORKSPACE_ERROR | Cannot switch to target workspace | Check workspace availability, try --no-workspace-switch |
| 4 | TIMEOUT | Operation exceeded time limit | Retry with longer timeout, check system performance |
| 5 | GENERAL_ERROR | Other system or application error | Check system logs, restart application |

## Backward Compatibility

### Existing Command Preservation
- All existing `window-manager` commands remain unchanged
- Existing output formats preserved for current commands
- No breaking changes to existing API or CLI interface

### New Command Integration
- New focus commands use consistent argument patterns
- JSON output follows existing schema patterns
- Error handling follows established CLI conventions

## Performance Contracts

### Response Time Requirements
- Handle validation: < 500ms (per SC-003)
- Current workspace focus: < 1 second (per SC-001)
- Cross-workspace focus: < 2 seconds (per SC-002)

### Resource Constraints
- Memory usage: < 50MB additional for focus operations
- CPU usage: < 10% sustained during operation
- Handle cache: Maximum 10,000 entries

## Security Considerations

### Input Validation
- All handle inputs validated for format and range
- Protection against handle enumeration attacks
- Rate limiting for focus requests (max 10/second)

### Permission Requirements
- Same permission model as existing window enumeration
- No elevation of privileges required beyond current implementation
- Graceful degradation when permissions insufficient

## Testing Contracts

### Unit Test Coverage
- Handle validation logic: 100% code coverage
- Error condition handling: All error paths tested
- Platform-specific implementations: Each platform tested independently

### Integration Test Scenarios
- Cross-workspace focus operations
- Handle validation edge cases
- Performance requirement validation
- Error recovery testing

### Acceptance Test Criteria
- All user scenarios from feature specification pass
- Performance requirements met on target platforms
- Error messages clear and actionable for users