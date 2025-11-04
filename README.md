# Window Manager with Focus Control

A cross-platform C++17 application for enumerating, searching, filtering, and focusing system windows with high performance and interactive capabilities.

## Features

- **Cross-platform window enumeration** - Supports Windows, macOS, and Linux (X11)
- **Direct window focus by handle** - Focus any window by providing its platform-specific handle
- **Cross-workspace window focus** - Automatically switch workspaces/desktops to focus windows
- **Comprehensive handle validation** - Robust validation with detailed error messages
- **Fast keyword search** - Real-time window filtering with case-sensitive/insensitive options
- **Interactive terminal UI** - FTXUI-based interface with live search and auto-refresh
- **Multiple output formats** - Text and JSON output support
- **Performance optimized** - Handles 50+ windows efficiently with caching and smart memory management
- **Rate limiting and timeout protection** - Enterprise-grade reliability features
- **Comprehensive error handling** - Platform-specific guidance and verbose debugging

## Supported Platforms

| Platform | Requirements | API Used |
|----------|-------------|----------|
| **Windows** | Windows Vista+ | Win32 API |
| **macOS** | macOS 10.12+ | Core Graphics + Accessibility |
| **Linux** | X11 Server | X11 API |

## Quick Start

### Prerequisites

- **C++17** compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- **CMake 3.16+**
- Platform-specific dependencies:
  - **macOS**: Xcode Command Line Tools
  - **Linux**: X11 development packages (`libx11-dev`, `libxtst-dev`)
  - **Windows**: Visual Studio 2017+ or MinGW

### Build Instructions

```bash
# Clone and navigate to project
git clone <repository-url>
cd cua

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build the project
cmake --build . --config Release

# The executable will be in build/bin/
```

### Platform-Specific Setup

#### macOS Setup
**IMPORTANT**: Grant Accessibility permissions for full functionality:

1. Open **System Preferences** > **Security & Privacy** > **Privacy**
2. Select **Accessibility** from the left panel
3. Click the lock icon (admin password required)
4. Add the application and check the enable box
5. Restart the application

#### Linux Setup
Ensure X11 is running and properly configured:

```bash
# Check X11 is available
echo $DISPLAY  # Should show :0 or similar

# Install required X11 packages (Ubuntu/Debian)
sudo apt-get install libx11-dev libxtst-dev

# For SSH users, enable X11 forwarding
ssh -X user@hostname
```

#### Windows Setup
No additional setup required. May need administrator privileges for some system windows.

## Usage

### Command Line Interface

#### List All Windows
```bash
# Basic listing
./window-manager list

# Show window handles for focus operations
./window-manager list --show-handles

# Compact handle listing (handles and titles only)
./window-manager list --handles-only

# Verbose output with performance stats
./window-manager list --verbose

# JSON format output
./window-manager list --format json

# Combined options
./window-manager list --show-handles --verbose --format json
```

#### Search Windows
```bash
# Basic keyword search
./window-manager search chrome

# Case-sensitive search
./window-manager search "Google Chrome" --case-sensitive

# Verbose search with timing info
./window-manager search firefox --verbose

# JSON output for search results
./window-manager search terminal --format json
```

#### Focus Windows by Handle
```bash
# Focus a window by its handle (with automatic workspace switching)
./window-manager focus a968

# Focus with verbose progress output
./window-manager focus a968 --verbose

# Focus without workspace switching (current workspace only)
./window-manager focus a968 --no-workspace-switch

# Focus with custom timeout (in seconds)
./window-manager focus a968 --timeout 10

# JSON output for focus operations
./window-manager focus a968 --format json
```

#### Validate Window Handles
```bash
# Validate a window handle
./window-manager validate-handle a968

# Detailed validation with JSON output
./window-manager validate-handle a968 --format json --verbose
```

#### Interactive Mode
```bash
# Start interactive filtering interface
./window-manager interactive

# Interactive mode ignores format option (uses FTXUI)
./window-manager interactive --format json  # Note: format ignored
```

### Interactive Mode Controls

Once in interactive mode:

- **Type** to search windows in real-time
- **F5** - Manual refresh window list
- **C** - Toggle case sensitivity
- **ESC** or **Q** - Quit to command line

### Output Examples

#### Text Format
```
Window List and Filter Program
==============================

Found 8 windows:

[1] Google Chrome - GitHub
    Process: chrome (1234)
    Bounds: 100,100 800x600
    Visible: Yes

[2] Terminal - bash
    Process: Terminal (5678)
    Bounds: 200,200 1024x768
    Visible: Yes
```

#### JSON Format
```json
{
  "windows": [
    {
      "id": 1,
      "title": "Google Chrome - GitHub",
      "processName": "chrome",
      "processId": 1234,
      "bounds": {
        "x": 100,
        "y": 100,
        "width": 800,
        "height": 600
      },
      "visible": true,
      "handle": "0x12345"
    }
  ],
  "totalCount": 8,
  "enumerationTime": "45ms",
  "platform": "macOS Core Graphics Enumerator (macOS 14.0) [Accessibility enabled]"
}
```

### Search Results
```bash
$ ./window-manager search "chrome" --verbose
Debug: Starting search for 'chrome'
Debug: Case sensitive: no
Debug: Search completed in 12ms
Debug: Found 3 matches out of 15 total windows

Search Results for "chrome" (3 matches):
=========================================

[1] Google Chrome - GitHub
    Process: chrome (1234)
    Match: Title contains "chrome"

[2] Chrome Remote Desktop
    Process: chrome (5678)
    Match: Title contains "chrome"

Search Performance: 12ms (meets <1s target)

Verbose Information:
- Platform: macOS Core Graphics Enumerator (macOS 14.0) [Accessibility enabled]
- Performance meets requirements: yes
- Cache enabled: yes
```

## Advanced Usage

### Window Focus Operations

#### Performance Requirements
- **Current workspace focus**: < 1 second
- **Cross-workspace focus**: < 2 seconds
- **Handle validation**: < 0.5 seconds
- **Rate limiting**: Maximum 10 focus requests per second

#### Focus Workflow
```bash
# Step 1: List windows with handles
./window-manager list --handles-only

# Step 2: Validate a handle before focusing
./window-manager validate-handle a968

# Step 3: Focus the window
./window-manager focus a968 --verbose
```

### Performance Monitoring
```bash
# Check if performance requirements are met
./window-manager list --verbose

# Test focus performance
./window-manager focus a968 --verbose
# Expected: "Performance requirement met (1000ms threshold)"

# Expected output includes:
# "Performance requirements met"
# "Enumeration time: XXms (target: <3000ms)"
# "Window count: XX (supports 50+ requirement)"
```

### Error Handling and Debugging
```bash
# Get platform-specific help
./window-manager --platform-help

# Common error scenarios:
./window-manager list  # Permission denied -> Shows platform-specific guidance
./window-manager search test --verbose  # Debugging failed searches

# Focus operation debugging
./window-manager validate-handle 12345 --verbose  # Check handle validity
./window-manager focus 12345 --verbose  # Debug focus operations
./window-manager focus 12345 --no-workspace-switch  # Test without workspace switching
```

### Window Focus Error Scenarios
```bash
# Invalid handle format
./window-manager focus "invalid"
# Output: ✗ Handle is invalid: invalid
#         Reason: Handle must be numeric (platform-specific window ID)

# Handle validation timeout
./window-manager validate-handle 12345 --timeout 1
# Output: ✗ Handle is invalid: 12345
#         Reason: Handle format invalid or window not found

# Focus with workspace switching disabled
./window-manager focus a968 --no-workspace-switch
# Output: ✗ Failed to focus window
#         Handle: a968
#         Error: Window is in different workspace
#         Suggestion: Try without --no-workspace-switch option
```

### Integration Examples

#### JSON Processing with jq
```bash
# Extract just window titles
./window-manager list --format json | jq -r '.windows[].title'

# Find windows by process
./window-manager list --format json | jq '.windows[] | select(.processName == "chrome")'

# Count visible windows
./window-manager list --format json | jq '[.windows[] | select(.visible == true)] | length'
```

#### Scripting Examples
```bash
#!/bin/bash
# Find and focus a specific window (pseudo-code)
WINDOW_ID=$(./window-manager search "Terminal" --format json | jq -r '.windows[0].handle')
if [ "$WINDOW_ID" != "null" ]; then
    echo "Found Terminal window: $WINDOW_ID"
    # Additional focus logic would go here
fi
```

## Architecture

### Core Components

```
src/
├── core/
│   ├── window.hpp          # WindowInfo data structure
│   ├── enumerator.hpp      # Platform abstraction layer
│   ├── window_manager.hpp  # Main facade with caching
│   └── exceptions.hpp      # Error handling
├── platform/
│   ├── windows/            # Win32 implementation
│   ├── macos/              # Core Graphics implementation
│   └── linux/              # X11 implementation
├── filters/
│   ├── search_query.hpp    # Search criteria and matching
│   ├── filter_result.hpp   # Results with performance metrics
│   └── filter.hpp          # Filtering logic with caching
└── ui/
    ├── cli.hpp             # Command-line interface
    └── interactive.hpp     # FTXUI terminal interface
```

### Performance Features

- **Smart caching** - Window lists cached for 5 seconds with thread-safe invalidation
- **Memory management** - Cache size limited to 10,000 windows, automatic cleanup
- **Optimized sorting** - Uses `stable_sort` for large datasets (>100 windows)
- **Vector reservation** - Pre-allocates memory based on expected window counts
- **Background refresh** - Interactive mode refreshes without blocking UI

### Success Criteria

✅ **SC-001**: Current workspace focus completes in <1 second
✅ **SC-002**: Cross-workspace focus completes in <2 seconds
✅ **SC-003**: Handle validation completes in <0.5 seconds
✅ **SC-004**: Window enumeration completes in <3 seconds
✅ **SC-005**: Keyword filtering completes in <1 second
✅ **SC-006**: Interactive mode provides real-time feedback
✅ **SC-007**: Cross-platform compatibility (Windows/Linux/macOS)
✅ **SC-008**: Supports 50+ windows without performance degradation

## Troubleshooting

### Common Issues

#### Permission Errors

**macOS**: "Permission Error: Cannot access window information"
```bash
# Solution: Grant Accessibility permissions
./window-manager --platform-help  # Shows detailed steps
```

**Linux**: "Permission Error: X11 connection failed"
```bash
# Check X11 environment
echo $DISPLAY
xhost +local:

# For SSH users
ssh -X username@hostname
```

**Windows**: "Permission Error: Access denied"
```bash
# Run as administrator
# Right-click application -> "Run as administrator"
```

#### Performance Issues

**Slow enumeration** (>3 seconds):
- Close unnecessary applications
- Check system resources (CPU/Memory)
- Try `--verbose` flag for detailed timing

**High memory usage**:
- Cache is limited to 10,000 windows automatically
- Disable caching if needed (not exposed in CLI but available in API)

#### Platform Issues

**Linux X11**: "Platform Error: X11 not available"
```bash
# Install X11 packages
sudo apt-get install libx11-dev libxtst-dev

# Check X11 server
ps aux | grep Xorg
```

**macOS**: "Platform Error: Core Graphics failed"
- Update to macOS 10.12+
- Grant Accessibility permissions
- Restart application after permission grant

### Getting Help

```bash
# Show usage and examples
./window-manager --help

# Platform-specific guidance
./window-manager --platform-help

# Version and build information
./window-manager --version

# Verbose debugging for any command
./window-manager [command] --verbose
```

## Development

### Building from Source

```bash
# Debug build for development
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .

# Run tests (if available)
ctest --output-on-failure

# Clean build
cmake --build . --target clean
```

### Dependencies

- **FTXUI** - Terminal UI library (automatically fetched by CMake)
- **Platform APIs** - Native window management APIs
  - Windows: `user32.dll`, `dwmapi.dll`
  - macOS: `ApplicationServices.framework`, `Carbon.framework`
  - Linux: `libX11`, `libXtst`

### Contributing

1. Ensure cross-platform compatibility
2. Follow C++17 standards
3. Add appropriate error handling
4. Include performance tests for large window counts
5. Update documentation for new features

## License

[Add your license information here]

## Changelog

### v2.0.0 - Window Focus Edition
- **NEW**: Direct window focus by handle with cross-workspace support
- **NEW**: Comprehensive handle validation with detailed error messages
- **NEW**: Window handle display options (--show-handles, --handles-only)
- **NEW**: Focus operation tracking and history
- **NEW**: Rate limiting and timeout protection for enterprise reliability
- **Enhanced**: Cross-platform workspace switching capabilities
- **Enhanced**: Error handling with recovery suggestions
- **Enhanced**: Performance monitoring for all operations

### v1.0.0
- Initial release with cross-platform window enumeration
- Keyword search and filtering
- Interactive terminal UI
- JSON output support
- Performance optimizations for 50+ windows
- Comprehensive error handling and platform guidance