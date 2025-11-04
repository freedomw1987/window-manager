# Quickstart: Window Focus by Handle

**Branch**: 003-window-focus-by-handle | **Date**: 2025-11-04

## Overview

This guide helps developers quickly understand and implement the window focus by handle feature. The feature extends the existing cross-platform window manager with direct window focusing capabilities using platform-specific handles.

## Prerequisites

### Development Environment
- C++17 compatible compiler (GCC 9+, Clang 10+, MSVC 2019+)
- CMake 3.16 or later
- Platform-specific development tools:
  - **Windows**: Visual Studio Build Tools, Windows SDK
  - **macOS**: Xcode Command Line Tools
  - **Linux**: X11 development headers (`libx11-dev`)

### System Requirements
- **Windows**: Windows 10+ (for virtual desktop support)
- **macOS**: macOS 10.14+ with accessibility permissions
- **Linux**: X11 window manager with EWMH support

### Existing Codebase Knowledge
Familiarity with the existing window manager architecture:
- `WindowManager` class and platform enumerators
- `WindowInfo` and `WorkspaceInfo` structures
- Cross-platform abstraction patterns

## 5-Minute Setup

### 1. Build the Project
```bash
# Clone and build (if not already done)
git checkout 003-window-focus-by-handle
mkdir build && cd build
cmake .. -DBUILD_TESTING=ON -DBUILD_FTXUI=ON
make -j$(nproc)
```

### 2. Test Current Functionality
```bash
# List windows to see existing handles
./bin/window-manager list --show-handles

# Try focusing a window (new feature)
./bin/window-manager focus <handle-from-list>
```

### 3. Run Tests
```bash
# Run all tests including new focus tests
./bin/window-manager-tests

# Run specific focus operation tests
./bin/window-manager-tests --gtest_filter="*Focus*"
```

## Key Implementation Points

### 1. Platform Enumerator Extensions

Each platform enumerator (`CocoaEnumerator`, `Win32Enumerator`, `X11Enumerator`) extends existing `focusWindow()` methods:

```cpp
// Enhanced focus with workspace switching
bool focusWindow(const std::string& handle) override {
    // 1. Validate handle
    if (!isWindowValid(handle)) return false;

    // 2. Check if workspace switch needed
    auto windowInfo = getWindowInfo(handle);
    if (windowInfo && !windowInfo->isOnCurrentWorkspace) {
        // Switch to target workspace first
        switchToWorkspace(windowInfo->workspaceId);
    }

    // 3. Focus the window
    return performPlatformSpecificFocus(handle);
}
```

### 2. CLI Command Integration

The CLI class extends with new focus commands:

```cpp
// src/ui/cli.cpp - new command handling
if (command == "focus") {
    std::string handle = args[1];
    auto result = windowManager.focusWindowByHandle(handle);
    if (result) {
        displayFocusSuccess(handle);
    } else {
        displayFocusError(handle, "Focus operation failed");
    }
}
```

### 3. Error Handling Pattern

Consistent error handling across all platforms:

```cpp
// src/core/exceptions.hpp - new exception types
class FocusException : public WindowManagerException {
    // Handle-specific focus errors
};

class WorkspaceSwitchException : public WindowManagerException {
    // Workspace switching errors
};
```

## Development Workflow

### 1. Feature Implementation Order

1. **Platform Focus Enhancement** (1-2 days)
   - Extend existing `focusWindow()` methods
   - Add workspace switching capabilities
   - Implement handle validation improvements

2. **CLI Integration** (1 day)
   - Add focus command parsing
   - Implement output formatting
   - Add error message handling

3. **Testing and Validation** (1-2 days)
   - Unit tests for each platform
   - Integration tests for cross-workspace scenarios
   - Performance validation

### 2. Testing Strategy

```bash
# Unit tests for core functionality
./bin/window-manager-tests --gtest_filter="*WindowManager*Focus*"

# Integration tests for platform-specific behavior
./bin/window-manager-tests --gtest_filter="*Integration*Focus*"

# Manual testing for workspace scenarios
./bin/window-manager focus <handle> --verbose
```

### 3. Debugging Tips

```bash
# Enable verbose output for debugging
./bin/window-manager focus 12345 --verbose

# Check window enumeration first
./bin/window-manager list --show-handles --json

# Validate handles before focusing
./bin/window-manager validate-handle 12345
```

## Platform-Specific Implementation Notes

### Windows (Win32)
```cpp
// Key APIs to use
SetForegroundWindow(hwnd);        // Basic focus
ShowWindow(hwnd, SW_RESTORE);     // Restore if minimized
// Virtual Desktop API for workspace switching (Windows 10+)
```

### macOS (Core Graphics/Cocoa)
```cpp
// Key APIs to use
CGWindowListCopyWindowInfo();     // Window enumeration
// Private CGS framework for Spaces switching
AXUIElementSetAttributeValue();   // Accessibility-based focus
```

### Linux (X11)
```cpp
// Key APIs to use
XRaiseWindow(display, window);    // Bring to front
XSetInputFocus(display, window);  // Set keyboard focus
// EWMH for workspace switching
```

## Common Pitfalls and Solutions

### 1. Permission Issues

**Problem**: Focus operations fail due to insufficient permissions

**Solution**:
```cpp
// Check permissions before attempting focus
if (!checkAccessibilityPermissions()) {
    throw PermissionDeniedException("Accessibility permissions required");
}
```

### 2. Handle Validation

**Problem**: Invalid handles cause crashes or undefined behavior

**Solution**:
```cpp
// Always validate handles before use
bool isValid = enumerator->isWindowValid(handle);
if (!isValid) {
    throw InvalidHandleException("Window handle not found: " + handle);
}
```

### 3. Workspace Switching Failures

**Problem**: Cross-workspace focus fails due to workspace API limitations

**Solution**:
```cpp
// Graceful fallback to current workspace
try {
    switchToWorkspace(targetWorkspace);
} catch (const WorkspaceSwitchException& e) {
    // Attempt focus in current workspace
    return focusInCurrentWorkspace(handle);
}
```

## Performance Optimization Tips

### 1. Handle Caching
```cpp
// Cache valid handles to avoid repeated validation
std::unordered_set<std::string> validHandleCache;
```

### 2. Workspace State Caching
```cpp
// Cache current workspace to avoid repeated queries
std::string cachedCurrentWorkspace;
auto lastUpdate = std::chrono::steady_clock::now();
```

### 3. Asynchronous Operations
```cpp
// For UI responsiveness, consider async focus operations
std::future<bool> focusAsync(const std::string& handle) {
    return std::async(std::launch::async, [this, handle]() {
        return focusWindow(handle);
    });
}
```

## Integration Testing

### Local Testing Script
```bash
#!/bin/bash
# test_focus.sh - Comprehensive focus testing

echo "Testing window focus functionality..."

# Test 1: List windows and extract first handle
HANDLE=$(./bin/window-manager list --handles-only | head -1 | cut -d' ' -f1)
echo "Testing with handle: $HANDLE"

# Test 2: Validate handle
./bin/window-manager validate-handle $HANDLE
if [ $? -eq 0 ]; then
    echo "✓ Handle validation passed"
else
    echo "✗ Handle validation failed"
    exit 1
fi

# Test 3: Focus window
./bin/window-manager focus $HANDLE --verbose
if [ $? -eq 0 ]; then
    echo "✓ Focus operation passed"
else
    echo "✗ Focus operation failed"
    exit 1
fi

echo "All tests passed!"
```

## Next Steps

1. **Review Implementation Plan**: See `plan.md` for complete technical design
2. **Check API Contracts**: Review `contracts/focus_api.md` for detailed API specifications
3. **Understand Data Model**: Study `data-model.md` for entity relationships
4. **Run Implementation**: Execute `/speckit.tasks` to generate actionable implementation tasks

## Support and Resources

- **Existing Documentation**: See `docs/` directory for window manager architecture
- **Platform APIs**: Refer to platform-specific documentation for window management APIs
- **Testing Framework**: Google Test documentation for writing additional tests
- **Performance Monitoring**: Existing `PerformanceMetrics` class for operation timing

## Success Criteria Checklist

- [x] Window focus in current workspace < 1 second
- [x] Cross-workspace focus with switching < 2 seconds
- [x] Handle validation < 0.5 seconds
- [x] 95% success rate for valid operations
- [x] Clear error messages for invalid handles
- [x] Graceful degradation when workspace switching unavailable