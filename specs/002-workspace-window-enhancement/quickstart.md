# Quickstart: Workspace Window Enhancement

**Feature**: Workspace Window Enhancement
**Target Audience**: Developers implementing the enhanced window management
**Prerequisites**: Familiarity with existing codebase, C++17, CMake

## Overview

This guide provides step-by-step instructions for implementing workspace-aware window management that extends the existing functionality with cross-platform virtual desktop support and enhanced search capabilities.

## Development Setup

### 1. Environment Requirements

**All Platforms**:
- C++17 compatible compiler
- CMake 3.16 or higher
- Git for version control

**Windows**:
- Visual Studio 2019+ or Clang
- Windows SDK 10.0.19041.0+
- COM support enabled

**macOS**:
- Xcode 12+ or Clang 10+
- macOS 10.15+ deployment target
- Accessibility permissions for testing

**Linux**:
- GCC 8+ or Clang 8+
- X11 development libraries
- EWMH-compliant window manager

### 2. Build Configuration

Update `CMakeLists.txt` to include enhanced dependencies:

```cmake
# Enhanced platform-specific libraries
if(WIN32)
    set(PLATFORM_LIBS user32 kernel32 psapi shell32 ole32 oleaut32)
elseif(APPLE)
    find_library(ACCESSIBILITY_LIBRARY ApplicationServices)
    set(PLATFORM_LIBS ${COREGRAPHICS_LIBRARY} ${FOUNDATION_LIBRARY}
                      ${COCOA_LIBRARY} ${CARBON_LIBRARY} ${ACCESSIBILITY_LIBRARY})
elseif(UNIX)
    find_package(X11 REQUIRED)
    set(PLATFORM_LIBS ${X11_LIBRARIES} ${X11_Xext_LIB})
endif()
```

### 3. Feature Branch Setup

```bash
# Ensure you're on the feature branch
git checkout 002-workspace-window-enhancement

# Verify current state
git status
git log --oneline -5
```

## Implementation Roadmap

### Phase 1: Core Data Structure Enhancement (Week 1) ✅ COMPLETED

#### 1.1 Update WindowInfo Structure

**File**: `src/core/window.hpp`

```cpp
// Add new fields to existing WindowInfo struct
struct WindowInfo {
    // ... existing fields ...

    // NEW: Workspace information
    std::string workspaceId;
    std::string workspaceName;
    bool isOnCurrentWorkspace;

    // NEW: Enhanced state
    WindowState state;
    bool isFocused;
    bool isMinimized;

    // Updated constructor
    WindowInfo(/* existing params */,
               const std::string& workspaceId = "",
               const std::string& workspaceName = "",
               bool onCurrentWorkspace = true,
               WindowState state = WindowState::Normal);
};
```

#### 1.2 Implement Enhanced JSON Output

**File**: `src/core/window.cpp`

```cpp
std::string WindowInfo::toJson() const {
    // Extend existing JSON with new fields
    // Maintain backward compatibility
}
```

#### 1.3 Create WorkspaceInfo Structure

**File**: `src/core/workspace.hpp` (new file)

```cpp
namespace WindowManager {
    struct WorkspaceInfo {
        std::string id;
        std::string name;
        int index;
        bool isCurrent;
        // ... implementation
    };
}
```

### Phase 2: Platform-Specific Workspace Integration (Week 2-3) ✅ COMPLETED

#### 2.1 Windows Implementation

**File**: `src/platform/windows/win32_enumerator.cpp`

```cpp
class Win32Enumerator : public WindowEnumerator {
private:
    IVirtualDesktopManager* m_pDesktopManager;

public:
    // Implement workspace enumeration
    std::vector<WorkspaceInfo> enumerateWorkspaces() override;
    WindowInfo createEnhancedWindowInfo(HWND hwnd);
    bool initializeVirtualDesktopManager();
};
```

**Key Implementation Points**:
- Initialize COM interface `IVirtualDesktopManager`
- Use `GetWindowDesktopId()` for workspace identification
- Implement graceful fallback when VD not supported
- Handle focus detection with `GetForegroundWindow()`

#### 2.2 macOS Implementation

**File**: `src/platform/macos/cocoa_enumerator.cpp`

```cpp
class CocoaEnumerator : public WindowEnumerator {
private:
    bool m_accessibilityEnabled;

public:
    std::vector<WorkspaceInfo> enumerateWorkspaces() override;
    WindowInfo createEnhancedWindowInfo(CGWindowID windowId, CFDictionaryRef windowInfo);
    bool checkAccessibilityPermissions();
    std::optional<WindowInfo> getFocusedWindowViaAccessibility();
};
```

**Key Implementation Points**:
- Use `CGWindowListCopyWindowInfo` as primary enumeration
- Implement Accessibility API integration for focus detection
- Handle workspace detection limitations gracefully
- Request permissions appropriately

#### 2.3 Linux Implementation

**File**: `src/platform/linux/x11_enumerator.cpp`

```cpp
class X11Enumerator : public WindowEnumerator {
private:
    Display* m_display;
    std::map<Atom, std::string> m_ewmhAtoms;

public:
    std::vector<WorkspaceInfo> enumerateWorkspaces() override;
    WindowInfo createEnhancedWindowInfo(Window window);
    bool initializeEWMHSupport();
    int getWindowDesktop(Window window);
};
```

**Key Implementation Points**:
- Initialize EWMH atoms for workspace queries
- Use `_NET_NUMBER_OF_DESKTOPS`, `_NET_CURRENT_DESKTOP`
- Implement robust window state detection
- Handle different window manager capabilities

### Phase 3: Enhanced Search Implementation (Week 3) ✅ COMPLETED

#### 3.1 Search Query Enhancement

**File**: `src/filters/search_query.hpp`

```cpp
enum class SearchField { Title, Owner, Both };

struct SearchQuery {
    std::string query;
    SearchField field;
    bool caseSensitive;
    std::string workspaceFilter;

    bool matches(const WindowInfo& window) const;
};
```

#### 3.2 Filter Result Enhancement

**File**: `src/filters/filter_result.cpp`

```cpp
struct FilterResult {
    std::vector<WindowInfo> windows;
    std::map<std::string, std::vector<WindowInfo>> byWorkspace;

    void groupByWorkspace();
    std::string toEnhancedOutput() const;
};
```

### Phase 4: CLI Interface Integration (Week 4) ✅ COMPLETED

#### 4.1 Enhanced CLI Commands

**File**: `src/ui/cli.cpp`

```cpp
class CLI {
public:
    int handleListCommand(const std::vector<std::string>& args);
    int handleSearchCommand(const std::vector<std::string>& args);
    int handleWorkspacesCommand(const std::vector<std::string>& args);
    int handleInfoCommand(const std::vector<std::string>& args);

private:
    void printWorkspaceHeader(const WorkspaceInfo& workspace);
    void printEnhancedWindowInfo(const WindowInfo& window);
};
```

#### 4.2 Backward Compatibility Layer

**File**: `src/ui/compatibility.cpp` (new file)

```cpp
namespace Compatibility {
    // Ensure old command formats continue to work
    std::vector<WindowInfo> stripWorkspaceInfo(const std::vector<WindowInfo>& windows);
    std::string toLegacyJson(const std::vector<WindowInfo>& windows);
}
```

## Testing Strategy

### 1. Unit Tests

**Directory**: `tests/unit/`

```cpp
// Core functionality tests
TEST(WindowInfoTest, WorkspaceFieldsInitialization)
TEST(SearchQueryTest, DualFieldMatching)
TEST(WorkspaceInfoTest, ValidationRules)

// Platform-specific tests
TEST(Win32EnumeratorTest, VirtualDesktopIntegration)
TEST(CocoaEnumeratorTest, AccessibilityPermissions)
TEST(X11EnumeratorTest, EWMHPropertyHandling)
```

### 2. Integration Tests

**Directory**: `tests/integration/`

```cpp
// Cross-platform workspace tests
TEST(WorkspaceEnumerationTest, AllPlatforms)
TEST(WindowStateDetectionTest, FocusAndMinimized)
TEST(SearchFunctionalityTest, TitleAndOwnerSearch)

// CLI integration tests
TEST(CLITest, BackwardCompatibility)
TEST(CLITest, EnhancedOutput)
```

### 3. Manual Testing Checklist

#### Windows Testing
- [ ] Test with Virtual Desktops enabled/disabled
- [ ] Verify focus detection across desktops
- [ ] Test with multiple monitors
- [ ] Validate COM interface error handling

#### macOS Testing
- [ ] Test with/without Accessibility permissions
- [ ] Verify Mission Control integration
- [ ] Test across different macOS versions
- [ ] Validate fallback behavior

#### Linux Testing
- [ ] Test across GNOME, KDE, Xfce
- [ ] Verify EWMH compliance detection
- [ ] Test with tiling window managers (i3)
- [ ] Validate X11 connection handling

## Performance Optimization

### 1. Caching Strategy

```cpp
class WindowCache {
private:
    std::chrono::steady_clock::time_point lastUpdate;
    std::vector<WindowInfo> cachedWindows;
    std::vector<WorkspaceInfo> cachedWorkspaces;

public:
    bool needsRefresh() const;
    void invalidate();
};
```

### 2. Lazy Loading

```cpp
class LazyWorkspaceEnumerator {
public:
    std::vector<WindowInfo> getWindowsOnDemand(const std::string& workspaceId);
    void prefetchCurrentWorkspace();
};
```

### 3. Performance Monitoring

```cpp
class PerformanceProfiler {
public:
    void startTimer(const std::string& operation);
    std::chrono::milliseconds endTimer(const std::string& operation);
    std::string getReport() const;
};
```

## Debugging and Troubleshooting

### 1. Debug Logging

```cpp
// Enable debug output
#ifdef DEBUG_WORKSPACE
#define WM_DEBUG(msg) std::cerr << "[WM DEBUG] " << msg << std::endl
#else
#define WM_DEBUG(msg)
#endif
```

### 2. Platform Detection

```cpp
// Verify platform capabilities at runtime
void validatePlatformSupport() {
    WM_DEBUG("Platform: " << getPlatformName());
    WM_DEBUG("Workspace support: " << (isWorkspaceSupported() ? "Yes" : "No"));
    WM_DEBUG("Permission status: " << getPermissionStatus());
}
```

### 3. Common Issues

#### Windows
- **Issue**: COM initialization fails
- **Solution**: Ensure `CoInitialize()` called before `IVirtualDesktopManager`

#### macOS
- **Issue**: Accessibility permissions denied
- **Solution**: Request permissions with proper dialog, provide fallback

#### Linux
- **Issue**: EWMH properties not available
- **Solution**: Detect window manager capabilities, implement fallbacks

## Deployment Checklist

### Pre-Release Testing
- [x] All unit tests pass on all platforms
- [x] Integration tests pass with real workspace scenarios
- [x] Performance benchmarks meet requirements (<3s enumeration)
- [x] Backward compatibility verified with existing clients
- [x] Error handling tested for edge cases

### Release Validation
- [x] CLI help documentation updated
- [x] JSON schema documented for API consumers
- [x] Performance regression testing completed
- [x] Multi-platform builds successful
- [x] Accessibility compliance verified (macOS)

## Next Steps

After completing the implementation:

1. **Performance Tuning**: Profile and optimize critical paths
2. **Documentation**: Update user guides and API documentation
3. **Integration**: Test with existing tools and scripts
4. **Feedback**: Gather user feedback for iterative improvements
5. **Future Enhancements**: Plan advanced features (window management, notifications)

## Support and Resources

### Code References
- **Existing Implementation**: `specs/001-window-list-filter/`
- **Research Documentation**: `specs/002-workspace-window-enhancement/research.md`
- **API Contracts**: `specs/002-workspace-window-enhancement/contracts/`

### External Documentation
- **Windows**: [Virtual Desktop API Documentation](https://docs.microsoft.com/en-us/windows/win32/api/shobjidl_core/nn-shobjidl_core-ivirtualdesktopmanager)
- **macOS**: [Core Graphics Window Services](https://developer.apple.com/documentation/coregraphics/quartz_window_services)
- **Linux**: [Extended Window Manager Hints](https://specifications.freedesktop.org/wm-spec/wm-spec-latest.html)

This quickstart provides a comprehensive roadmap for implementing the workspace window enhancement while maintaining the existing functionality and ensuring cross-platform compatibility.