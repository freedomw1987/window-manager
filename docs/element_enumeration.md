# Element Enumeration Documentation

**Feature**: Window-Specific Element Operations
**Version**: 1.0.0
**Last Updated**: 2025-11-05

## Overview

The window manager now supports UI element enumeration within specific windows using the `--window <handle>` parameter. This feature allows you to discover and search for UI elements (buttons, text fields, labels, etc.) inside application windows.

## Basic Usage

### List Elements in a Window

```bash
# First, get a window handle
window-manager list --show-handles

# Then enumerate elements in that window
window-manager list --window <handle>
```

### Search Elements in a Window

```bash
# Search for specific elements within a window
window-manager search "button" --window <handle>
window-manager search "OK" --window <handle> --case-sensitive
```

## Command Reference

### List Command with Element Enumeration

```bash
window-manager list --window <handle> [options]
```

**Options:**
- `--window <handle>` - Enumerate elements within the specified window
- `--verbose` - Show detailed information including performance stats
- `--format json` - Output results in JSON format
- `--show-handles` - Include element handles in output
- `--handles-only` - Show only handles and types (compact format)

### Search Command with Element Enumeration

```bash
window-manager search <keyword> --window <handle> [options]
```

**Options:**
- `--window <handle>` - Search elements within the specified window
- `--case-sensitive` - Perform case-sensitive search
- `--verbose` - Show detailed search information
- `--format json` - Output results in JSON format

## Platform Support

### Windows
- **Technology**: Microsoft UI Automation (UIA)
- **Requirements**: Windows Vista or later
- **Capabilities**: Full element tree access with semantic information
- **Permissions**: May require administrator privileges for some system windows

### macOS
- **Technology**: Core Graphics + NSAccessibility APIs
- **Requirements**: macOS Sierra (10.12) or later
- **Capabilities**: Full accessibility tree access
- **Permissions**: Requires explicit accessibility permissions:
  1. System Preferences → Security & Privacy → Privacy → Accessibility
  2. Add the application and enable it
  3. Restart the application

### Linux
- **Technology**: AT-SPI (Assistive Technology Service Provider Interface) with X11 fallback
- **Requirements**: X11 server, AT-SPI services
- **Capabilities**: Variable depending on application accessibility support
- **Permissions**: May need X11 permissions for some applications

## Performance Characteristics

- **Target**: <2 seconds for windows with up to 100 elements
- **Caching**: 30-second TTL for element trees
- **Memory**: Estimated usage displayed in verbose mode
- **Timeout**: Automatic 2-second timeout for enumeration operations

## Element Types

The system recognizes the following UI element types:

- **Button** - Clickable buttons
- **TextField** - Text input fields
- **Label** - Static text labels
- **Menu** - Menu containers and items
- **ComboBox** - Dropdown selections
- **CheckBox** - Toggle checkboxes
- **RadioButton** - Radio button selections
- **Image** - Image elements
- **Link** - Clickable links
- **Table** - Table containers and cells
- **Window** - Sub-windows and dialogs
- **Pane** - Container panels
- **ScrollBar** - Scroll controls
- **Slider** - Range sliders
- **ProgressBar** - Progress indicators

## Error Handling

### Common Error Messages

```bash
# Invalid window handle
Error: Invalid window handle: 12345
Error: Use 'window-manager list --show-handles' to see available windows

# Platform not supported
Error: Element enumeration is not supported on this platform
Platform: Linux X11

# Permission denied
Error: Element enumeration requires accessibility permissions

# Window doesn't support elements
Error: Window does not support element enumeration
```

### Troubleshooting

1. **Handle Validation Errors**
   - Verify the window handle exists: `window-manager list --show-handles`
   - Check if the window is still open and accessible

2. **Permission Errors**
   - **Windows**: Run as administrator if needed
   - **macOS**: Grant accessibility permissions in System Preferences
   - **Linux**: Check X11 permissions and AT-SPI services

3. **Platform Support Issues**
   - Verify your OS is supported (Windows Vista+, macOS 10.12+, Linux with X11)
   - Check that required services are running (AT-SPI on Linux)

4. **Performance Issues**
   - Use `--verbose` to see timing information
   - Large windows with many elements may exceed the 2-second target
   - Check system resources if enumeration is consistently slow

## Examples

### Complete Workflow

```bash
# 1. List all windows to find target application
window-manager list --show-handles

# Output:
# [1] Handle: d3d0
#     Title: Calculator
#     Owner: Calculator
#     Position: (100, 100)  Size: 320x480  PID: 1234

# 2. Enumerate elements in the Calculator window
window-manager list --window d3d0

# 3. Search for specific elements
window-manager search "=" --window d3d0
window-manager search "button" --window d3d0 --verbose

# 4. Get detailed information in JSON format
window-manager list --window d3d0 --format json --verbose
```

### JSON Output Format

```json
{
  "windowHandle": "d3d0",
  "totalElements": 25,
  "enumerationTime": "150ms",
  "platform": "Windows UIA",
  "elements": [
    {
      "handle": "d3d0-btn-1",
      "type": "Button",
      "name": "1",
      "position": {"x": 50, "y": 200},
      "size": {"width": 60, "height": 40},
      "state": "Normal",
      "visible": true,
      "enabled": true
    }
  ]
}
```

## Integration with Automation Tools

The element enumeration feature is designed to support automation and testing scenarios:

1. **Test Automation**: Locate specific UI elements for interaction
2. **Accessibility Testing**: Verify element accessibility properties
3. **UI Inspection**: Debug and analyze application interfaces
4. **Screen Readers**: Support assistive technology integration

## Development Notes

- Element handles are platform-specific and may change between sessions
- Caching improves performance but may show stale data for rapidly changing UIs
- The feature maintains backward compatibility - existing commands work unchanged
- Performance monitoring helps identify applications with complex UI structures

For additional support or to report issues, see the main project documentation.