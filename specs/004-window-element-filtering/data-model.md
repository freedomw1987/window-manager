# Data Model: Window-Specific Element Operations

**Feature**: Window-Specific Element Operations
**Date**: 2025-11-05
**Status**: Complete

## Overview

This document defines the data structures and relationships required for UI element enumeration within specific windows, extending the existing window management system.

## Core Entities

### UIElement

Represents a single UI element within an application window.

```cpp
namespace WindowManager {

enum class ElementType {
    Unknown,
    Button,
    TextField,
    Label,
    Menu,
    MenuItem,
    ComboBox,
    CheckBox,
    RadioButton,
    Image,
    Link,
    Table,
    Cell,
    Window,
    Pane,
    ScrollBar,
    Slider,
    ProgressBar
};

enum class ElementState {
    Normal,
    Disabled,
    Hidden,
    Focused,
    Selected,
    Checked,
    Expanded,
    Collapsed
};

struct UIElement {
    // Identity
    std::string handle;                 // Platform-specific element identifier
    std::string parentWindowHandle;     // Reference to containing window
    std::string parentElementHandle;    // Reference to parent element (for hierarchy)

    // Basic Properties
    ElementType type;
    std::string name;                   // Element name/title
    std::string value;                  // Element value/text content
    std::string description;            // Accessibility description
    std::string role;                   // Accessibility role

    // Position and Size (relative to window)
    int x, y;                          // Position relative to parent window
    unsigned int width, height;       // Element dimensions
    bool isVisible;                    // Element visibility state

    // State Information
    ElementState state;
    bool isEnabled;
    bool isFocusable;
    bool isClickable;

    // Accessibility Properties
    std::string accessibilityLabel;
    std::string accessibilityHelp;
    std::string accessibilityValue;

    // Platform-specific metadata (JSON string for flexibility)
    std::string platformData;

    // Timestamps
    std::chrono::steady_clock::time_point discoveredAt;

    // Constructors
    UIElement();
    UIElement(const std::string& handle, const std::string& parentWindow,
              ElementType type, const std::string& name);

    // Validation
    bool isValid() const;
    bool hasValidPosition() const;
    bool hasValidDimensions() const;

    // Output formatting
    std::string toString() const;
    std::string toJson() const;
    std::string toCompactString() const;

    // Comparison operators
    bool operator==(const UIElement& other) const;
    bool operator<(const UIElement& other) const;
};

} // namespace WindowManager
```

### ElementEnumerationResult

Container for element enumeration results with metadata.

```cpp
namespace WindowManager {

struct ElementEnumerationResult {
    // Results
    std::vector<UIElement> elements;
    std::string windowHandle;
    std::string windowTitle;

    // Metadata
    size_t totalElementCount;
    size_t filteredElementCount;
    std::chrono::milliseconds enumerationTime;
    std::chrono::steady_clock::time_point enumeratedAt;

    // Status information
    bool success;
    std::string errorMessage;
    bool hasAccessibilityPermissions;
    bool supportsElementEnumeration;

    // Performance metrics
    bool meetsPerformanceTarget() const {
        return enumerationTime <= std::chrono::milliseconds(2000);
    }

    // Output formatting
    std::string getSummary() const;
    std::string toJson() const;
};

} // namespace WindowManager
```

### ElementSearchQuery

Search criteria for filtering elements within windows.

```cpp
namespace WindowManager {

enum class ElementSearchField {
    Name,
    Value,
    Description,
    Type,
    All
};

struct ElementSearchQuery {
    std::string searchTerm;
    ElementSearchField field;
    bool caseSensitive;
    bool exactMatch;
    std::vector<ElementType> typeFilter;  // Filter by element types
    bool includeHidden;                   // Include hidden elements
    bool includeDisabled;                 // Include disabled elements

    // Constructor
    ElementSearchQuery(const std::string& term,
                      ElementSearchField field = ElementSearchField::All,
                      bool caseSensitive = false,
                      bool exactMatch = false);

    // Validation
    bool isValid() const;

    // Matching logic
    bool matches(const UIElement& element) const;

    // String representation
    std::string toString() const;
};

} // namespace WindowManager
```

## Extended Entities

### Enhanced WindowInfo

Extension to existing WindowInfo to support element enumeration.

```cpp
namespace WindowManager {

// Addition to existing WindowInfo struct
struct WindowInfo {
    // ... existing fields ...

    // NEW: Element enumeration support
    bool supportsElementEnumeration;     // Platform capability
    size_t estimatedElementCount;        // Performance hint
    std::chrono::steady_clock::time_point lastElementEnumeration;

    // NEW: Element enumeration metadata
    bool hasElementCache;                // Whether elements are cached
    std::chrono::milliseconds lastElementEnumerationTime;

    // NEW: Methods for element support
    bool canEnumerateElements() const;
    bool hasRecentElementCache(std::chrono::milliseconds maxAge = std::chrono::seconds(30)) const;
};

} // namespace WindowManager
```

## Relationships

### Entity Relationships

```text
WindowInfo (1) ────┐
                   │
                   ├─► UIElement (0..*)
                   │   │
                   │   └─► UIElement (0..*) [parent-child hierarchy]
                   │
                   └─► ElementEnumerationResult (0..*)
                       │
                       └─► UIElement (0..*)

ElementSearchQuery ────┐
                       │
                       └─► ElementEnumerationResult (1)
                           │
                           └─► UIElement (0..*)
```

### Data Flow

1. **Element Discovery**:
   ```
   WindowHandle → ElementEnumerator → ElementEnumerationResult → UIElement[]
   ```

2. **Element Search**:
   ```
   WindowHandle + ElementSearchQuery → ElementEnumerator → ElementEnumerationResult → UIElement[]
   ```

3. **Hierarchical Navigation**:
   ```
   UIElement → parentElementHandle → UIElement (parent)
   UIElement → childElements → UIElement[] (children)
   ```

## Validation Rules

### UIElement Validation
- `handle` must be non-empty
- `parentWindowHandle` must reference valid window
- `type` must be valid ElementType
- `x, y` can be negative (elements can be outside visible area)
- `width, height` must be > 0 for visible elements
- Platform-specific handle format validation

### ElementSearchQuery Validation
- `searchTerm` must be non-empty for text searches
- `typeFilter` must contain valid ElementType values
- Field-specific validation based on `field` selection

### ElementEnumerationResult Validation
- `windowHandle` must reference valid window
- `totalElementCount >= filteredElementCount`
- `enumerationTime` must be positive
- `success == true` implies `elements` contains valid data

## State Transitions

### Element Discovery Lifecycle
```text
Unknown → Discovered → Cached → Stale → Rediscovered
    ↓         ↓          ↓        ↓         ↓
   N/A    Enumerated  Available  Expired  Updated
```

### Element State Changes
```text
Normal ⟷ Focused ⟷ Selected
  ↓         ↓         ↓
Disabled  Hidden   Checked/Unchecked
```

## Performance Considerations

### Caching Strategy
- Cache elements per window handle with 30-second TTL
- Invalidate cache on window state changes
- Progressive loading for large element trees
- Memory limits: max 1000 elements per window

### Optimization Targets
- Element enumeration: < 2 seconds for 100 elements
- Search operations: < 500ms for cached results
- Memory usage: < 50MB for element cache
- Cache hit ratio: > 80% for repeated operations

## Platform-Specific Considerations

### Windows (Win32)
- Handle format: IUIAutomationElement pointer as string
- Maximum element depth: 20 levels
- Performance: UIA tree traversal can be slow for deep hierarchies

### macOS (Accessibility)
- Handle format: AXUIElementRef as CFTypeID string
- Requires accessibility permissions
- Performance: Generally fast but permission-dependent

### Linux (X11/AT-SPI)
- Handle format: AT-SPI object path
- Variable support depending on application
- Fallback to basic window property inspection