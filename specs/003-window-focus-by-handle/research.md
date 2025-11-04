# Research: Window Focus by Handle

**Branch**: 003-window-focus-by-handle | **Date**: 2025-11-04

## Executive Summary

The existing codebase already contains most infrastructure needed for window focus by handle functionality. The core window management, platform abstraction, and focus methods are implemented. The main gaps are workspace switching capabilities and user interface integration for handle-based focus commands.

## Research Findings

### 1. Window Focus Implementation Status

**Decision**: Extend existing `focusWindow()` methods in platform enumerators
**Rationale**: All three platform enumerators (Win32, Cocoa, X11) already implement `focusWindow(const std::string& handle)` methods
**Alternatives considered**: Creating new focus interfaces was rejected due to existing working implementations

**Current Implementation Analysis**:
- `CocoaEnumerator::focusWindow()` - Uses CGWindowList API and process activation (lines 108+ in cocoa_enumerator.cpp)
- `Win32Enumerator::focusWindow()` - Uses SetForegroundWindow API (win32_enumerator.cpp:72)
- `X11Enumerator::focusWindow()` - Uses XRaiseWindow and XSetInputFocus (x11_enumerator.cpp:143)

### 2. Workspace Switching Capabilities

**Decision**: Implement workspace switching as new functionality in platform enumerators
**Rationale**: Current focus methods don't handle cross-workspace scenarios - they only focus windows in current workspace
**Alternatives considered**: Automatic workspace detection was rejected due to complexity and user control requirements

**Research Gap Identified**: Need to research platform-specific workspace/virtual desktop switching APIs:
- **macOS**: Mission Control/Spaces API via CGS (Core Graphics Services) private framework
- **Windows**: Virtual Desktop APIs (Windows 10+ VirtualDesktopManager)
- **Linux**: EWMH (Extended Window Manager Hints) for workspace switching

### 3. Window Handle Validation and Error Handling

**Decision**: Enhance existing `isWindowValid()` methods for robust handle validation
**Rationale**: Current codebase has basic validation but needs strengthening for user-input scenarios
**Alternatives considered**: Creating separate validation was rejected to maintain consistency with existing patterns

**Current Status**: Basic validation exists in all platforms - need to enhance for edge cases like:
- Closed windows with stale handles
- Permission-restricted windows
- Cross-workspace window handle validity

### 4. User Interface Integration

**Decision**: Extend existing CLI interface with new focus commands
**Rationale**: CLI class already supports workspace-aware operations and error display methods
**Alternatives considered**: Separate focus UI was rejected to maintain unified command interface

**Implementation Approach**:
- Add `focus` command to CLI argument parsing
- Utilize existing error display methods (`displayError`, `displaySuccess`)
- Leverage existing workspace display helpers for user feedback

### 5. Performance Considerations

**Decision**: Use existing performance monitoring framework
**Rationale**: `PerformanceMetrics` structure and timing infrastructure already supports focus operation requirements (<1s current workspace, <2s cross-workspace)
**Alternatives considered**: New performance tracking was rejected due to existing comprehensive monitoring

## Technical Implementation Strategy

### Phase 1: Core Focus Enhancement
1. Extend platform `focusWindow()` methods to handle cross-workspace scenarios
2. Add workspace switching capabilities to each platform enumerator
3. Enhance handle validation in existing `isWindowValid()` methods

### Phase 2: User Interface Integration
1. Add focus command parsing to CLI interface
2. Integrate workspace switching feedback using existing display methods
3. Add comprehensive error handling for invalid handles

### Phase 3: Testing and Validation
1. Extend existing Google Test suite with focus operation tests
2. Add cross-workspace integration tests
3. Performance validation against existing SC-001/SC-002 criteria

## Dependencies and Constraints

### External Dependencies
- **macOS**: Private CGS framework for Spaces control (requires careful implementation due to private API usage)
- **Windows**: Windows 10+ for Virtual Desktop APIs (graceful degradation for older versions)
- **Linux**: EWMH-compliant window manager (most modern WMs support this)

### Technical Constraints
- Must maintain existing performance thresholds (3s max enumeration time)
- Cannot break existing window enumeration and filtering functionality
- Must handle graceful degradation when workspace switching is unavailable

## Risk Assessment

### Low Risk
- Core focus functionality (already implemented and tested)
- Handle validation enhancement (incremental improvement)
- CLI interface extension (follows existing patterns)

### Medium Risk
- Cross-workspace focus integration (new functionality, needs careful testing)
- Platform-specific workspace switching (requires platform-specific knowledge)

### High Risk
- macOS Spaces integration (private API usage, potential future compatibility issues)

## Next Steps

1. **Phase 1 Implementation**: Extend focus methods with workspace awareness
2. **Platform API Research**: Deep-dive into workspace switching APIs for each platform
3. **Interface Design**: Design CLI commands for handle-based focus operations
4. **Testing Strategy**: Develop comprehensive test cases for cross-workspace scenarios