# Research: Window-Specific Element Operations

**Date**: 2025-11-05
**Feature**: Window-Specific Element Operations
**Status**: Complete

## Research Scope

Investigation into extending the existing window manager to support UI element enumeration within specific windows, with particular focus on macOS implementation recommendations provided by the user.

## Key Research Findings

### 1. macOS Implementation Strategy (Based on User Recommendations)

**Decision**: Use AXUIElement (Accessibility API) for cross-process automation with C++ direct usage
**Rationale**: User input specifically recommends AXUIElement for cross-process automation/testing scenarios, noting that C++ can use it directly with proper permission handling.

**Implementation approach**:
- **For own apps**: Could use Cocoa (Objective-C++ bridge) but not applicable here since we're targeting external windows
- **For cross-process automation** (our use case): AXUIElement is the recommended approach
- **For window/screen info only**: CGWindowList/screenshots could work, but user notes that for element-level access, returning to Accessibility APIs is still recommended

**Key considerations from user input**:
- C++ can directly use AXUIElement APIs
- Permission handling is critical and must be implemented
- This is the stable and "proper" approach for cross-process element access

**Alternatives considered**:
- Cocoa (Objective-C++ bridge) - recommended for own apps, not applicable for cross-process
- CGWindowList/screenshots - insufficient for element-level detail

### 2. Cross-Platform API Strategy

**Decision**: Implement platform-specific element finders with unified interface
**Rationale**: Each platform has different APIs and capabilities, requiring tailored implementations while maintaining consistent user experience.

**Platform-specific approaches**:
- **macOS**: AXUIElement (Accessibility API) - direct C++ usage as recommended
- **Windows**: UI Automation API - established pattern for Windows element access
- **Linux**: AT-SPI (Assistive Technology Service Provider Interface) - standard for Linux accessibility

**Alternatives considered**:
- Single unified API wrapper - rejected due to significant platform differences in capabilities and permission models
- JavaScript/Electron wrapper - rejected to maintain performance and avoid additional runtime dependencies

### 3. Permission Handling Architecture

**Decision**: Implement platform-specific permission checking with clear user guidance
**Rationale**: Permission requirements vary significantly across platforms, especially critical for macOS as noted in user recommendations.

**macOS permission strategy** (based on user input):
- Implement runtime permission checking using AXIsProcessTrusted()
- Provide clear error messages directing users to System Preferences → Security & Privacy → Privacy → Accessibility
- Handle permission denial gracefully with helpful guidance
- Check permissions before attempting element operations

**Cross-platform patterns**:
- Windows: Check for UI Automation availability and handle elevated privilege scenarios
- Linux: Verify AT-SPI service availability and DBus connectivity
- All platforms: Provide platform-specific troubleshooting guidance

### 4. API Integration with Existing Codebase

**Decision**: Extend existing CMake/C++17 architecture with platform-specific modules
**Rationale**: Maintains consistency with current codebase while adding necessary platform capabilities.

**Integration approach**:
- Extend existing `src/platform/` structure with element enumeration capabilities
- Use existing CMake platform detection for conditional compilation
- Maintain existing error handling and CLI patterns
- Leverage current FTXUI integration for consistent output formatting

**Dependencies to add**:
- macOS: ApplicationServices.framework for AXUIElement
- Windows: oleaut32, ole32 for UI Automation
- Linux: libatspi2.0-dev for AT-SPI integration

### 5. CLI Command Extension Strategy

**Decision**: Add optional `--window <handle>` parameter to existing list and search commands
**Rationale**: Maintains backward compatibility while providing new functionality through opt-in parameter.

**Implementation approach**:
```cpp
// Enhanced function signatures
int listWindows(bool verbose, const std::string& format,
                bool showHandles, bool handlesOnly,
                const std::string& windowHandle = "");
int searchWindows(const std::string& keyword, bool caseSensitive,
                 bool verbose, const std::string& format,
                 const std::string& windowHandle = "");
```

**Command behavior**:
- Without `--window`: Existing behavior unchanged (backward compatibility)
- With `--window <handle>`: Switch to element enumeration mode for specified window
- Invalid handle: Clear error message with guidance to get valid handles

**Alternatives considered**:
- Separate element commands (e.g., `list-elements`) - rejected to maintain command simplicity
- Subcommands (e.g., `list window <handle>`) - rejected to maintain existing CLI patterns

### 6. Performance and Caching Strategy

**Decision**: Implement element-level caching with 30-second TTL
**Rationale**: Element enumeration is more expensive than window enumeration, especially with cross-process accessibility APIs.

**Performance targets** (from success criteria):
- <2 seconds for windows with up to 100 elements
- 50% faster search when scoped to specific window vs system-wide

**Caching strategy**:
- Cache element trees per window handle
- 30-second expiration to balance performance with accuracy
- Clear cache when window focus changes or handle becomes invalid
- Platform-specific optimizations (e.g., AXUIElement observation patterns on macOS)

### 7. Error Handling and User Experience

**Decision**: Implement comprehensive error handling with platform-specific guidance
**Rationale**: Accessibility APIs have complex permission and availability requirements that users need clear guidance to resolve.

**Error scenarios to handle**:
- Invalid window handles
- Missing accessibility permissions (critical for macOS)
- Platform API unavailability
- Window closed during enumeration
- Application not supporting accessibility

**User guidance strategy**:
- Platform-specific error messages with actionable steps
- Integration with existing help system
- Verbose mode for debugging accessibility issues

## Implementation Recommendations

### Phase 1: Core Infrastructure
1. Implement base element enumeration interface
2. Add platform detection for accessibility API availability
3. Extend CLI argument parsing for `--window` parameter

### Phase 2: Platform Implementation
1. **macOS**: Implement AXUIElement integration following user recommendations
2. **Windows**: Implement UI Automation element enumeration
3. **Linux**: Implement AT-SPI integration with X11 fallback

### Phase 3: Integration and Testing
1. Integrate element enumeration with existing list/search commands
2. Implement comprehensive error handling and permission checking
3. Add platform-specific testing for accessibility APIs

### Phase 4: Performance and Polish
1. Implement caching strategy for element data
2. Performance optimization to meet <2 second target
3. Documentation and user guidance for platform setup

## Conclusion

The research confirms that the user's macOS recommendations provide a solid foundation for implementing cross-process element enumeration. AXUIElement is indeed the appropriate choice for this use case, and the permission handling requirements are well understood. The extension can be implemented while maintaining the existing codebase architecture and backward compatibility.