# Quickstart: Window-Specific Element Operations

**Feature**: Window-Specific Element Operations
**Date**: 2025-11-05
**Status**: Design Phase

## Overview

This quickstart guide demonstrates how to use the new `--window <handle>` parameter with `list` and `search` commands to discover and filter UI elements within specific application windows.

## Prerequisites

### Platform Requirements
- **Windows**: Vista or later, optional administrator privileges for some applications
- **macOS**: Sierra 10.12+, accessibility permissions required
- **Linux**: X11 environment, AT-SPI2 support recommended

### Permissions Setup

#### macOS Accessibility Setup
```bash
# Check current accessibility permissions
./window-manager --platform-help

# Grant permissions through System Preferences:
# 1. Open System Preferences > Security & Privacy > Privacy
# 2. Select 'Accessibility' from left panel
# 3. Click lock and enter admin password
# 4. Add window-manager and enable checkbox
# 5. Restart application
```

#### Linux AT-SPI Setup
```bash
# Install AT-SPI2 if not available
sudo apt-get install at-spi2-core libatspi2.0-dev  # Ubuntu/Debian
sudo yum install at-spi2-core at-spi2-core-devel   # RHEL/CentOS

# Verify AT-SPI is running
ps aux | grep at-spi
```

## Basic Usage

### Step 1: Get Window Handles

First, list available windows to get window handles:

```bash
# List all windows with handles
./window-manager list --show-handles

# Example output:
# Handle    Title                     Process
# ------    -----                     -------
# 12345     Google Chrome - Gmail     chrome
# 67890     Terminal - bash           terminal
# 13579     Calculator                calculator
```

### Step 2: List Elements in Specific Window

Use the window handle to list all UI elements within that window:

```bash
# List all elements in Chrome window
./window-manager list --window 12345

# Verbose output with element details
./window-manager list --window 12345 --verbose

# JSON output for programmatic use
./window-manager list --window 12345 --format json
```

Example output:
```
Elements in window 12345 (Google Chrome - Gmail):

Handle    Type      Name                    Position    State
------    ----      ----                    --------    -----
12345-1   Button    "Send"                  (450,200)   Enabled
12345-2   TextField "Subject"               (100,150)   Focused
12345-3   TextField "Message body"          (100,180)   Enabled
12345-4   Link      "Inbox (3)"             (50,100)    Enabled
12345-5   Menu      "Gmail"                 (20,50)     Enabled

Found 23 elements in 1.2 seconds
```

### Step 3: Search Elements Within Window

Search for specific elements within the window:

```bash
# Search for buttons in Chrome window
./window-manager search "button" --window 12345

# Case-sensitive search for "Send" button
./window-manager search "Send" --window 12345 --case-sensitive

# Search in message text fields
./window-manager search "message" --window 12345 --verbose
```

Example output:
```
Search results for "Send" in window 12345:

Handle    Type      Name                    Value       Position
------    ----      ----                    -----       --------
12345-1   Button    "Send"                  ""          (450,200)
12345-15  Button    "Send & Archive"        ""          (350,200)

Found 2 matches out of 23 elements (search time: 0.3s)
```

## Advanced Usage

### Filtering by Element Type

```bash
# Search only in text fields
./window-manager search "password" --window 12345 --type textfield

# Search in buttons and links
./window-manager search "login" --window 12345 --type button,link
```

### Performance Monitoring

```bash
# Check element enumeration performance
./window-manager list --window 12345 --verbose

# Example performance output:
# Performance Statistics:
# - Element enumeration: 850ms (target: <2000ms) ✓
# - Total elements found: 45
# - Platform: macOS with Accessibility API
# - Cache status: Cold (first enumeration)
```

### Troubleshooting

```bash
# Validate window handle still exists
./window-manager validate-handle 12345

# Check platform capabilities
./window-manager --platform-help

# Test element access permissions
./window-manager list --window 12345 --verbose
```

## Common Workflows

### Automation Script Workflow

```bash
#!/bin/bash

# 1. Find target application window
CHROME_HANDLE=$(./window-manager search "Google Chrome" | grep -o "Handle: [0-9]*" | cut -d' ' -f2)

# 2. Verify window supports element enumeration
if ./window-manager list --window $CHROME_HANDLE >/dev/null 2>&1; then
    echo "Chrome window supports element enumeration"

    # 3. Find specific element (e.g., address bar)
    ADDRESS_BAR=$(./window-manager search "address" --window $CHROME_HANDLE --format json | jq -r '.elements[0].handle')

    echo "Found address bar element: $ADDRESS_BAR"
else
    echo "Chrome window does not support element enumeration"
fi
```

### Testing Interface Elements

```bash
# Test different element types in a window
for type in button textfield menu link; do
    echo "Testing $type elements:"
    ./window-manager search "$type" --window 12345 --format json | jq '.filteredElementCount'
done
```

### Performance Benchmarking

```bash
# Benchmark element enumeration across different windows
./window-manager list --show-handles | grep -v "Handle" | while read handle title rest; do
    echo "Benchmarking window: $title"
    time ./window-manager list --window $handle >/dev/null
done
```

## Output Formats

### Text Format (Default)
- Human-readable table format
- Highlighted search terms
- Performance statistics with verbose flag

### JSON Format
```bash
./window-manager list --window 12345 --format json
```

```json
{
  "window": {
    "handle": "12345",
    "title": "Google Chrome - Gmail"
  },
  "enumeration": {
    "success": true,
    "elementCount": 23,
    "enumerationTime": "1200ms",
    "timestamp": "2025-11-05T10:30:00Z"
  },
  "elements": [
    {
      "handle": "12345-1",
      "type": "Button",
      "name": "Send",
      "position": {"x": 450, "y": 200},
      "size": {"width": 80, "height": 30},
      "state": "Enabled",
      "accessible": true
    }
  ]
}
```

## Error Handling

### Common Errors and Solutions

#### Permission Denied
```bash
# Error: Permission denied accessing window elements
# Solution: Grant accessibility permissions (see Prerequisites)
```

#### Window Not Found
```bash
# Error: Window handle 12345 not found
# Solution: Refresh window list, window may have closed
./window-manager list --show-handles
```

#### Element Enumeration Not Supported
```bash
# Error: Window does not support element enumeration
# Solution: Application may not expose accessibility interface
# Fallback: Use window-level operations only
```

#### Performance Issues
```bash
# Warning: Element enumeration took 3500ms (exceeds 2000ms target)
# Solution: Application has many elements, consider filtering
./window-manager search "specific_term" --window 12345
```

## Next Steps

1. **For Automation**: Integrate window element discovery into test automation frameworks
2. **For Testing**: Use element enumeration to verify UI component accessibility
3. **For Development**: Build custom tools using JSON output for programmatic access
4. **For Accessibility**: Audit applications for accessibility compliance

## Support

- Run `./window-manager --help` for command reference
- Run `./window-manager --platform-help` for platform-specific guidance
- Use `--verbose` flag for detailed diagnostic information
- Check project documentation for advanced configuration options