# CLI Interface Contract: Workspace Window Enhancement

**Version**: 2.0.0
**Feature**: Workspace Window Enhancement
**Backward Compatibility**: Full compatibility with existing CLI commands

## Command Line Interface

### Enhanced Window Listing

#### Command: `window-manager list` (Enhanced)
**Purpose**: List all windows with workspace information

**Syntax**:
```bash
window-manager list [OPTIONS]
```

**Options**:
- `--workspace <id>` - Filter by workspace ID
- `--current-workspace` - Show only current workspace windows
- `--all-workspaces` - Show windows from all workspaces (default)
- `--format <text|json>` - Output format (default: text)
- `--show-state` - Include window state information (default: true)
- `--compact` - Use compact output format

**Output Examples**:

**Text Format (Enhanced)**:
```
Workspace: Desktop 1 (ID: 0, Current: Yes)
├── Chrome - Google                     [PID: 1234] [State: Focused]
├── Terminal                            [PID: 5678] [State: Normal]
└── Code - Visual Studio Code           [PID: 9012] [State: Normal]

Workspace: Desktop 2 (ID: 1, Current: No)
├── Slack                               [PID: 3456] [State: Minimized]
└── Notes                               [PID: 7890] [State: Normal]

Total: 5 windows across 2 workspaces
```

**JSON Format (Enhanced)**:
```json
{
  "metadata": {
    "totalWindows": 5,
    "totalWorkspaces": 2,
    "currentWorkspace": "0",
    "enumerationTime": 45,
    "platform": "Windows 11 with Virtual Desktop support"
  },
  "workspaces": [
    {
      "id": "0",
      "name": "Desktop 1",
      "index": 0,
      "isCurrent": true,
      "windowCount": 3,
      "windows": [
        {
          "handle": "1234567890",
          "title": "Chrome - Google",
          "x": 100, "y": 100, "width": 800, "height": 600,
          "isVisible": true,
          "processId": 1234,
          "ownerName": "chrome.exe",
          "workspaceId": "0",
          "workspaceName": "Desktop 1",
          "isOnCurrentWorkspace": true,
          "state": "Focused",
          "isFocused": true,
          "isMinimized": false
        }
      ]
    }
  ]
}
```

### Enhanced Search Functionality

#### Command: `window-manager search` (Enhanced)
**Purpose**: Search windows by title and/or owner name

**Syntax**:
```bash
window-manager search <query> [OPTIONS]
```

**Options**:
- `--field <title|owner|both>` - Search field (default: both)
- `--case-sensitive` - Enable case-sensitive search
- `--regex` - Use regular expression matching
- `--workspace <id>` - Limit search to specific workspace
- `--current-workspace` - Search only current workspace
- `--format <text|json>` - Output format

**Examples**:
```bash
# Search both title and owner (default behavior - BACKWARD COMPATIBLE)
window-manager search "chrome"

# Search only in application names
window-manager search "chrome" --field owner

# Search with regex
window-manager search "^Visual.*Code$" --regex

# Search on current workspace only
window-manager search "terminal" --current-workspace
```

**Output Format**:
```
Search Results: "chrome" in title and owner (47ms)

Found 3 windows:

Workspace: Desktop 1 (Current)
├── Chrome - Google Search              [chrome.exe] [PID: 1234] [Focused]
└── Chrome - GitHub                     [chrome.exe] [PID: 1234] [Normal]

Workspace: Desktop 2
└── Google Chrome                       [chrome.exe] [PID: 5678] [Minimized]

Summary: 3 matches across 2 workspaces
```

### Workspace Management Commands

#### Command: `window-manager workspaces`
**Purpose**: List and manage workspaces

**Syntax**:
```bash
window-manager workspaces [SUBCOMMAND]
```

**Subcommands**:
- `list` - List all workspaces (default)
- `current` - Show current workspace information
- `info <id>` - Show detailed workspace information

**Examples**:
```bash
# List all workspaces
window-manager workspaces
window-manager workspaces list

# Show current workspace
window-manager workspaces current

# Show specific workspace info
window-manager workspaces info 1
```

**Output**:
```
Workspaces (2 total):

* Desktop 1 (ID: 0) - 3 windows - Current
  Desktop 2 (ID: 1) - 2 windows

Current workspace: Desktop 1 (0)
Total windows: 5 across 2 workspaces
```

### Information and Status Commands

#### Command: `window-manager info`
**Purpose**: Show platform and capability information

**Output**:
```
Platform: Windows 11 (Build 22621)
Workspace Support: Yes (Virtual Desktops)
Window Manager: DWM (Desktop Window Manager)
Enhanced Features: Focus detection, State tracking, Workspace enumeration
Permissions: Standard (no elevation required)

Capabilities:
├── Window enumeration: ✓
├── Workspace enumeration: ✓
├── Focus detection: ✓
├── State detection: ✓
└── Search enhancement: ✓
```

#### Command: `window-manager status`
**Purpose**: Show current system status and window summary

**Output**:
```
System Status:
├── Current Workspace: Desktop 1 (ID: 0)
├── Total Workspaces: 2
├── Total Windows: 12
├── Current Workspace Windows: 5
├── Focused Window: Chrome - Google [PID: 1234]
└── Minimized Windows: 2

Last Update: 2025-10-26 14:30:15 (45ms)
```

## Backward Compatibility Guarantees

### Existing Commands (Unchanged Behavior)
```bash
# These commands work exactly as before
window-manager list
window-manager search "term"
window-manager --help
window-manager --version
```

### Output Format Compatibility

#### Text Output
- All existing text output preserved
- New workspace information added as additional sections
- Existing parsers continue to work

#### JSON Output
- All existing JSON fields preserved in exact same format
- New fields added to existing objects
- Existing JSON parsers ignore unknown fields gracefully

### Command Line Arguments
- All existing arguments preserved
- New arguments added with sensible defaults
- No breaking changes to argument parsing

## Enhanced Features (Additive)

### New Global Options
- `--workspace-info` - Include workspace information in output
- `--no-workspace-info` - Exclude workspace information (legacy mode)
- `--current-only` - Filter to current workspace across all commands

### Environment Variables
- `WINDOW_MANAGER_WORKSPACE_MODE=auto|enabled|disabled` - Control workspace features
- `WINDOW_MANAGER_OUTPUT_FORMAT=text|json|compact` - Default output format

### Exit Codes
```
0  - Success
1  - General error
2  - No windows found
3  - Search query invalid
4  - Workspace not supported
5  - Permission denied
10 - Platform not supported
```

## Performance Requirements

### Response Time Targets
- Window enumeration: < 3 seconds (all workspaces)
- Search operations: < 1 second
- Workspace enumeration: < 500ms
- Single window info: < 100ms

### Resource Usage
- Memory: < 50MB for normal operation
- CPU: Minimal impact during enumeration
- Permissions: No elevation required on any platform

### Caching Strategy
- Window list cached for 1 second
- Workspace list cached for 5 seconds
- Search results cached for 30 seconds
- Cache invalidation on workspace changes

## Error Handling

### Graceful Degradation
```bash
# If workspace support not available
window-manager list
# Output: Standard window list without workspace information
# Exit code: 0 (success)

# If search fails
window-manager search "nonexistent"
# Output: "No windows found matching 'nonexistent'"
# Exit code: 2
```

### Error Messages
- Clear, actionable error messages
- Suggest alternatives when features unavailable
- Include troubleshooting hints for permission issues

### Logging
- Debug logging via `--verbose` flag
- Error logging to stderr
- Performance metrics via `--timing` flag