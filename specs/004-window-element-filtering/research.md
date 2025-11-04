# Research: Window-Specific Element Operations

**Date**: 2025-11-05
**Feature**: Window-Specific Element Operations
**Status**: Complete

## Research Scope

Investigation into extending the existing window manager to support UI element enumeration within specific windows, adding `--window <handle>` parameters to `list` and `search` commands.

## Key Research Findings

### 1. Current Architecture Analysis

**Decision**: Current system enumerates windows, not UI elements within windows
**Rationale**: After analyzing the codebase (`src/core/window.hpp`, `src/core/enumerator.hpp`, `src/main.cpp`), the system currently:
- Enumerates application windows using platform APIs (Win32/Core Graphics/X11)
- Provides window metadata (title, position, size, process info)
- Supports window-level operations (focus, validation)
- Does NOT enumerate UI elements (buttons, text fields, etc.) within windows

**Alternatives considered**:
- Modifying existing window enumeration to include elements vs. adding new element enumeration subsystem
- Rejected window modification approach as it would break existing functionality and architectural separation

### 2. Platform Element Enumeration Research

**Decision**: Implement new element enumeration subsystem alongside existing window enumeration
**Rationale**: Each platform requires different APIs for UI element discovery:

#### Windows Platform (Win32)
- **API**: UI Automation (IUIAutomation) or Microsoft Active Accessibility (MSAA)
- **Implementation**: Use `IUIAutomationElement` interface to traverse element tree within window
- **Scope**: Can enumerate all control types (buttons, text fields, menus, etc.)
- **Performance**: Synchronous tree traversal, typically <100ms for standard windows

#### macOS Platform (Accessibility)
- **API**: Accessibility Inspector APIs (AXUIElement)
- **Implementation**: Use `AXUIElementCreateApplication()` and `AXUIElementCopyElementAtPosition()`
- **Scope**: Full accessibility tree access with element properties
- **Permissions**: Requires explicit accessibility permissions (already handled in existing code)

#### Linux Platform (X11)
- **API**: AT-SPI (Assistive Technology Service Provider Interface) or direct X11 property inspection
- **Implementation**: Limited native support; may need AT-SPI2 integration
- **Scope**: Variable depending on application accessibility support
- **Fallback**: Basic window content analysis via X11 properties

### 3. Integration Architecture

**Decision**: Create new `ElementEnumerator` subsystem parallel to existing `WindowEnumerator`
**Rationale**:
- Preserves existing window enumeration functionality
- Allows incremental development and testing
- Maintains clear separation of concerns
- Enables platform-specific optimizations

**Alternatives considered**:
- Extending `WindowEnumerator` directly - rejected due to complexity and scope creep
- Single unified interface - rejected due to different data models and performance characteristics

### 4. Command-Line Interface Extension

**Decision**: Add optional `--window <handle>` parameter to existing `list` and `search` commands
**Rationale**:
- Backward compatible: existing commands work unchanged
- Intuitive: parameter clearly indicates scoping operation
- Consistent: follows existing pattern of optional parameters
- Testable: can validate with/without parameter independently

**Implementation approach**:
```cpp
// In main.cpp argument parsing
std::string windowHandle; // Empty means system-wide (current behavior)
for (size_t i = 2; i < args.size(); ++i) {
    if (args[i] == "--window" && i + 1 < args.size()) {
        windowHandle = args[++i];
    }
    // ... other existing options
}

// Modified function signatures
int listWindows(bool verbose, const std::string& format,
                bool showHandles, bool handlesOnly,
                const std::string& windowHandle = "");
int searchWindows(const std::string& keyword, bool caseSensitive,
                 bool verbose, const std::string& format,
                 const std::string& windowHandle = "");
```

### 5. Data Model Design

**Decision**: Create new `UIElement` structure separate from `WindowInfo`
**Rationale**: UI elements have different properties and lifecycle than windows

**Key Properties**:
- Element handle/identifier
- Element type (button, text field, label, etc.)
- Position relative to window
- Text content (if applicable)
- Accessibility properties
- Parent window reference

### 6. Performance Considerations

**Decision**: Implement lazy loading and caching for element enumeration
**Rationale**: Element enumeration is more expensive than window enumeration

**Strategy**:
- Cache elements per window handle with TTL
- Progressive discovery (enumerate on-demand)
- Parallel enumeration for multiple windows
- Platform-specific optimizations

### 7. Error Handling Strategy

**Decision**: Graceful degradation when element access fails
**Rationale**: Some applications may restrict element access or lack accessibility support

**Fallback hierarchy**:
1. Full element enumeration (preferred)
2. Basic window content inspection
3. Window-only information with clear messaging
4. Error with helpful guidance

## Implementation Recommendations

### Phase 1: Core Infrastructure
1. Create `UIElement` data structure
2. Implement base `ElementEnumerator` interface
3. Add platform-specific element enumeration
4. Extend command-line argument parsing

### Phase 2: Integration
1. Modify `list` and `search` commands to accept window handle
2. Implement element filtering and search logic
3. Update CLI output formatting for elements
4. Add comprehensive error handling

### Phase 3: Optimization
1. Implement caching and performance optimizations
2. Add comprehensive testing for all platforms
3. Performance tuning to meet success criteria
4. Documentation and examples

## Conclusion

This feature represents a significant scope expansion from window management to UI element inspection. The research confirms feasibility across all target platforms with appropriate APIs available. The recommended approach maintains backward compatibility while adding powerful new capabilities for automation and testing scenarios.