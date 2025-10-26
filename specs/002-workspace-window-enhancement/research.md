# Research: Workspace Window Enhancement

**Feature**: Workspace Window Enhancement
**Created**: 2025-10-26
**Purpose**: Resolve platform-specific workspace API integration requirements

## Research Questions Resolved

### Windows Virtual Desktop Integration

**Decision**: Use documented IVirtualDesktopManager COM interface with fallback detection
**Rationale**:
- Documented APIs provide stability across Windows updates
- IVirtualDesktopManager offers sufficient functionality for window workspace detection
- Undocumented APIs provide full control but risk breaking with Windows updates
- Fallback detection ensures graceful degradation on unsupported systems

**Implementation Approach**:
- Primary: IVirtualDesktopManager COM interface (`CLSID_VirtualDesktopManager`)
- Methods: `GetWindowDesktopId()`, `IsWindowOnCurrentVirtualDesktop()`
- Window state: Standard Win32 APIs (`GetForegroundWindow()`, `IsIconic()`, `GetWindowPlacement()`)
- Error handling: Graceful fallback when virtual desktop APIs unavailable

**Alternatives Considered**:
- Undocumented COM interfaces (IVirtualDesktopManagerInternal) - Rejected due to stability concerns
- Registry-based detection - Rejected due to complexity and unreliability

### macOS Spaces/Mission Control Integration

**Decision**: Use CGWindowListCopyWindowInfo with Accessibility APIs and polling approach
**Rationale**:
- No official public API exists for Spaces enumeration
- CGWindowListCopyWindowInfo provides comprehensive window information
- Accessibility APIs enhance focus and state detection
- Private APIs (CGS*) are fragile and break across macOS versions

**Implementation Approach**:
- Primary: CGWindowListCopyWindowInfo for window enumeration
- Window state: kCGWindowIsOnscreen, window bounds analysis for minimized detection
- Focus detection: Accessibility API (AXFocusedApplication/AXFocusedWindow)
- Permissions: Require Accessibility permissions for enhanced functionality
- Polling: Periodic updates since real-time notifications aren't available

**Alternatives Considered**:
- Private CGS APIs (CGSGetActiveSpace, CGSGetSpacesForWindows) - Rejected due to version fragility
- NSWorkspace - Rejected due to limited workspace functionality

### Linux EWMH/X11 Integration

**Decision**: Use Extended Window Manager Hints (EWMH) with X11 property queries
**Rationale**:
- EWMH is the standard specification for window manager interaction
- Widely supported across major Linux desktop environments
- Provides comprehensive workspace and window state information
- Cross-window manager compatibility with proper fallbacks

**Implementation Approach**:
- Workspace enumeration: `_NET_NUMBER_OF_DESKTOPS`, `_NET_DESKTOP_NAMES`, `_NET_CURRENT_DESKTOP`
- Window-workspace mapping: `_NET_WM_DESKTOP` property
- Window state: `_NET_WM_STATE` with `_NET_WM_STATE_HIDDEN` for minimized detection
- Focus detection: Multiple methods (`_NET_ACTIVE_WINDOW`, `XGetInputFocus()`)
- Cross-WM support: Window manager detection and capability adaptation

**Alternatives Considered**:
- Direct window manager specific APIs - Rejected due to fragmentation
- Wayland protocols - Future consideration, X11 focus for current implementation

## Technology Integration Decisions

### Enhanced WindowInfo Structure

**Decision**: Extend existing WindowInfo struct with workspace and state fields
**Fields to Add**:
- `std::string workspaceId` - Platform-specific workspace identifier
- `std::string workspaceName` - Human-readable workspace name (if available)
- `WindowState state` - Enumeration for focused/minimized/normal states
- `bool isOnCurrentWorkspace` - Quick check for current workspace membership

### Search Enhancement Strategy

**Decision**: Extend existing search functionality to match both title and owner transparently
**Rationale**:
- Maintains backward compatibility with existing search commands
- Users don't need to learn new syntax or flags
- Implementation leverages existing search infrastructure

**Implementation Approach**:
- Modify search query processing to check both `title` and `ownerName` fields
- Use case-insensitive matching consistent with existing behavior
- Combine results from both fields, avoiding duplicates
- Maintain existing output format with enhanced information display

### Performance Optimization

**Decision**: Implement caching and incremental updates where possible
**Approaches**:
- Windows: Cache virtual desktop information, refresh on desktop change events
- macOS: Polling-based updates with configurable intervals
- Linux: Event-driven updates using X11 PropertyNotify events
- All platforms: Window list caching with invalidation on window create/destroy

### Error Handling Strategy

**Decision**: Graceful degradation with feature detection
**Levels**:
1. **Full functionality**: All workspace features available
2. **Limited functionality**: Basic window enumeration without workspace information
3. **Fallback mode**: Current behavior maintained when workspace APIs unavailable

## Implementation Dependencies

### Build System Requirements
- Windows: Link with `shell32`, `ole32`, `oleaut32`, `psapi`
- macOS: Link with `CoreGraphics`, `Foundation`, `Cocoa`, `ApplicationServices`
- Linux: Link with `X11`, `Xext` libraries

### Permission Requirements
- Windows: No additional permissions required
- macOS: Accessibility permissions for enhanced focus detection
- Linux: X11 connection access (standard for desktop applications)

### Version Compatibility
- Windows: Windows 10 build 10240+ for virtual desktop support
- macOS: macOS 10.9+ for Core Graphics APIs, 10.15+ for enhanced Accessibility
- Linux: EWMH-compliant window managers (GNOME 3+, KDE 4+, Xfce 4.10+)

## Testing Strategy

### Platform-Specific Test Cases
- **Windows**: Test with/without virtual desktops enabled, multiple monitor setups
- **macOS**: Test with Mission Control disabled, different Space configurations
- **Linux**: Test across GNOME, KDE, Xfce, i3 window managers

### Cross-Platform Integration Tests
- Workspace enumeration accuracy
- Window state detection reliability
- Search functionality across title and owner fields
- Performance under high window count scenarios

### Backward Compatibility Tests
- Existing command-line interface preservation
- JSON output format compatibility
- Performance regression prevention