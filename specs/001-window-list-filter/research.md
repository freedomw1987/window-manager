# Research Findings: Window List and Filter Program

**Created**: 2025-10-26
**Context**: Technical research for C++ cross-platform window management application

## Platform-Specific APIs

### Decision: Platform-Specific API Strategy
**Rationale**: Create unified C++ interface with platform-specific implementations using native APIs for optimal performance and compatibility.

**Implementation Approach**:
- **Windows**: Win32 API (`EnumWindows`, `GetWindowText`, `GetWindowRect`)
- **Linux**: X11 with EWMH support (`XQueryTree`, `XGetWindowProperty`)
- **macOS**: Core Graphics (`CGWindowListCopyWindowInfo`)

**Alternatives considered**:
- Cross-platform GUI frameworks (Qt, GTK) - Rejected due to large dependencies and limited external window enumeration capabilities
- Single platform focus - Rejected to meet broader user needs

### Windows Implementation Details
- **Primary APIs**: `EnumWindows()`, `GetWindowText()`, `GetWindowRect()`, `IsWindowVisible()`
- **Libraries**: user32.lib, kernel32.lib, psapi.lib
- **Performance**: <100ms for 50+ windows, meets 3-second requirement easily
- **Special requirements**: None for basic enumeration

### Linux Implementation Details
- **Primary APIs**: `XQueryTree()` for enumeration, `XGetWindowProperty()` for EWMH properties
- **Libraries**: X11, with optional Xinerama for multi-monitor
- **Performance**: Potentially slower with deep hierarchies, cache window properties
- **Special requirements**: X11 server running, DISPLAY environment variable

### macOS Implementation Details
- **Primary APIs**: `CGWindowListCopyWindowInfo()` from Core Graphics
- **Libraries**: CoreGraphics.framework, Foundation.framework, Cocoa.framework
- **Performance**: <50ms typical, fastest of the three platforms
- **Special requirements**: Accessibility permissions required for comprehensive enumeration

## Testing Framework

### Decision: Google Test + Google Mock
**Rationale**: Superior mocking capabilities essential for testing platform-specific APIs without requiring GUI environments. Industry standard with excellent cross-platform support.

**Key advantages**:
- Sophisticated mocking of system APIs (Win32, X11, Core Graphics)
- Mature cross-platform support across all target platforms
- Excellent CI/CD integration and tooling
- Comprehensive documentation and community support

**Alternatives considered**:
- **Catch2**: Good modern C++ design but limited mocking capabilities
- **doctest**: Ultra-lightweight but insufficient mocking for system API testing

**Setup approach**:
- Use FetchContent for dependency management
- Platform-specific test configurations in CMake
- Mock interfaces for window enumeration APIs
- Headless testing strategies for CI environments

## Interactive UI Framework

### Decision: FTXUI (Primary) with Raw Terminal I/O (Fallback)
**Rationale**: FTXUI provides modern C++ terminal UI framework with excellent real-time input handling and cross-platform support, perfectly aligned with interactive filtering requirements.

**FTXUI advantages**:
- Native real-time input handling for interactive search
- Cross-platform compatibility (Windows, Linux, macOS)
- Modern C++ design with component-based architecture
- Performance characteristics meet <1s filtering requirement
- Reasonable dependency footprint

**Raw Terminal I/O fallback**:
- Minimal dependencies approach
- Platform-specific terminal control sequences
- Greater implementation complexity but maximum control
- Suitable for scenarios requiring absolute minimal dependencies

**Alternatives considered**:
- **ncurses**: Mature but Windows compatibility issues
- **CLI11/argparse**: Excellent for traditional CLI but not real-time interaction
- **ImTUI**: Immediate mode approach but less mature ecosystem

## Build System Architecture

### Decision: CMake with Platform Detection
**Rationale**: Industry standard with excellent cross-platform support and conditional compilation capabilities.

**Configuration strategy**:
```cmake
# Platform detection and conditional sources
if(WIN32)
    # Win32-specific sources and libraries
elseif(APPLE)
    # macOS-specific sources and frameworks
elseif(UNIX)
    # X11-specific sources and libraries
endif()
```

**Key features**:
- Automatic platform detection and configuration
- Conditional compilation for platform-specific code
- Integrated testing with CTest
- Modern CMake practices (3.16+ requirements)

## Performance Optimization Strategy

### Decision: Multi-layered Caching with Background Updates
**Rationale**: Meets aggressive performance requirements while maintaining responsive user experience.

**Implementation approach**:
- **Initial enumeration**: Cache all window data on startup
- **Background refresh**: Periodic updates (1-2 second intervals)
- **Incremental filtering**: In-memory string matching operations
- **Memory management**: Efficient string handling for large window lists

**Performance targets validation**:
- ✅ Window enumeration <3 seconds: All platforms easily achieve <1 second
- ✅ Search filtering <1 second: In-memory operations typically <50ms
- ✅ Support 50+ windows: All platforms handle efficiently

## Cross-Platform Compatibility Strategy

### Decision: Interface Abstraction with Platform Factories
**Rationale**: Clean separation of concerns while maintaining platform-specific optimization.

**Architecture**:
```cpp
class WindowEnumerator {
public:
    virtual std::vector<WindowInfo> enumerate() = 0;
    static std::unique_ptr<WindowEnumerator> create();
};
```

**Benefits**:
- Testable design with mockable interfaces
- Platform-specific optimizations preserved
- Unified API for application logic
- Easy addition of new platforms

## Deployment Considerations

### Decision: Single Binary Distribution
**Rationale**: Simplifies distribution while maintaining cross-platform compatibility.

**Approach**:
- Static linking where possible
- Runtime platform detection
- Graceful degradation for missing features
- Clear error messages for permission issues (especially macOS)

**Platform-specific notes**:
- **Windows**: Standard executable, no special requirements
- **Linux**: X11 dependency, handle missing display gracefully
- **macOS**: Prompt for accessibility permissions, provide user guidance

## Security and Permissions

### Decision: Minimal Permissions with User Guidance
**Rationale**: Balance functionality with user privacy and system security.

**Implementation**:
- Request only necessary permissions
- Provide clear explanation of permission needs
- Graceful degradation when permissions denied
- No persistence of window data beyond session

**Platform considerations**:
- **macOS**: Accessibility permission required, user prompt with explanation
- **Windows**: No special permissions for basic enumeration
- **Linux**: Standard user access sufficient for X11

This research provides the technical foundation for implementing a robust, cross-platform window management application that meets all specification requirements while maintaining clean architecture and optimal performance.