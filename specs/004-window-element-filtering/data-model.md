# Data Model: Window-Specific Element Operations

**Feature**: Window-Specific Element Operations
**Date**: 2025-11-05
**Status**: Complete

## Overview

This document defines the data structures and relationships required for UI element enumeration within specific windows, extending the existing window management system with cross-platform accessibility support.

## Core Entities

### UIElement

Represents a single UI element within an application window, discovered through platform-specific accessibility APIs.

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
    // Identification
    std::string handle;                    // Platform-specific element identifier
    std::string parentWindowHandle;        // Window handle this element belongs to
    ElementType type;                      // Type of UI element

    // Properties
    std::string name;                      // Element name/title (from accessibility)
    std::string description;               // Element description (from accessibility)
    std::string value;                     // Element value/content (if applicable)
    std::string className;                 // Platform-specific class name

    // Geometry
    int x, y;                             // Position relative to screen
    int width, height;                    // Element dimensions
    bool isVisible;                       // Visibility state

    // State
    ElementState state;                   // Current element state
    bool isEnabled;                       // Whether element accepts interaction
    bool isFocusable;                     // Whether element can receive focus

    // Hierarchy
    std::string parentElementHandle;      // Parent element (empty if direct window child)
    std::vector<std::string> childHandles; // Child element handles

    // Platform-specific data
    std::string platformInfo;            // Platform-specific metadata

    // Discovery metadata
    std::chrono::steady_clock::time_point discoveredAt; // When element was found
    std::string enumerationMethod;       // How element was discovered ("AXUIElement", "UIA", "AT-SPI")
};

} // namespace WindowManager
```

### ElementEnumerationResult

Contains the result of element enumeration operations, including discovered elements and operation metadata.

```cpp
namespace WindowManager {

struct ElementEnumerationResult {
    // Request context
    std::string windowHandle;             // Target window handle
    std::string requestId;                // Unique request identifier

    // Results
    std::vector<UIElement> elements;      // Discovered elements
    size_t totalElementCount;             // Total number of elements found
    size_t filteredElementCount;          // Number after filtering (if applicable)

    // Operation metadata
    bool success;                         // Whether enumeration succeeded
    std::string errorMessage;             // Error description (if failed)
    std::chrono::milliseconds enumerationTime; // Time taken for operation

    // Platform information
    std::string platformMethod;          // Platform API used ("AXUIElement", "UIA", "AT-SPI")
    std::string platformVersion;         // Platform API version info
    bool hasAccessibilityPermissions;    // Whether required permissions are granted

    // Performance data
    bool meetsPerformanceTarget() const {
        return enumerationTime.count() <= 2000; // <2 seconds target
    }

    // Validation
    bool isValid() const {
        return success && !windowHandle.empty() && totalElementCount == elements.size();
    }

    // Factory methods
    static ElementEnumerationResult createSuccess(const std::string& windowHandle,
                                                std::vector<UIElement> elements,
                                                std::chrono::milliseconds duration,
                                                const std::string& platformMethod);

    static ElementEnumerationResult createError(const std::string& windowHandle,
                                              const std::string& errorMessage,
                                              const std::string& platformMethod);
};

} // namespace WindowManager
```

### ElementSearchQuery

Defines search criteria for finding specific elements within a window.

```cpp
namespace WindowManager {

enum class ElementSearchField {
    Name,              // Search in element name/title
    Description,       // Search in element description
    Value,            // Search in element value/content
    ClassName,        // Search in element class name
    All               // Search in all text fields
};

struct ElementSearchQuery {
    // Search criteria
    std::string searchTerm;               // Text to search for
    ElementSearchField searchField;       // Which field(s) to search
    bool caseSensitive;                   // Case-sensitive matching
    bool exactMatch;                      // Exact vs partial matching

    // Filtering options
    std::vector<ElementType> includeTypes; // Only search these element types (empty = all)
    std::vector<ElementType> excludeTypes;  // Exclude these element types
    bool includeHidden;                   // Include hidden elements
    bool includeDisabled;                 // Include disabled elements

    // Result options
    size_t maxResults;                    // Maximum number of results (0 = unlimited)
    bool includeHierarchy;                // Include parent/child relationships in results

    // Constructors
    ElementSearchQuery(const std::string& term,
                      ElementSearchField field = ElementSearchField::All,
                      bool caseSensitive = false,
                      bool exactMatch = false)
        : searchTerm(term), searchField(field), caseSensitive(caseSensitive),
          exactMatch(exactMatch), includeHidden(false), includeDisabled(true),
          maxResults(0), includeHierarchy(false) {}

    // Validation
    bool isValid() const {
        return !searchTerm.empty() && maxResults >= 0;
    }

    // Matching logic
    bool matches(const UIElement& element) const;
};

} // namespace WindowManager
```

## Relationships

### Window-Element Relationship

- **One-to-Many**: Each window handle can contain multiple UI elements
- **Hierarchical**: Elements can have parent-child relationships within the window
- **Temporal**: Element handles may become invalid when windows close or change

```cpp
// Relationship management
class WindowElementManager {
public:
    // Get all elements for a window
    std::vector<UIElement> getElementsForWindow(const std::string& windowHandle);

    // Get element hierarchy
    std::vector<UIElement> getElementHierarchy(const std::string& elementHandle);

    // Validate element still exists
    bool isElementValid(const std::string& elementHandle);

    // Clear elements when window closes
    void invalidateElementsForWindow(const std::string& windowHandle);
};
```

### Element Hierarchy

Elements within a window form a tree structure representing the UI hierarchy.

```cpp
struct ElementHierarchy {
    UIElement root;                       // Root element (usually the window itself)
    std::map<std::string, UIElement> elements; // All elements by handle
    std::map<std::string, std::vector<std::string>> children; // Parent -> children mapping

    // Navigation helpers
    std::vector<UIElement> getChildren(const std::string& parentHandle) const;
    UIElement getParent(const std::string& childHandle) const;
    std::vector<UIElement> getDescendants(const std::string& ancestorHandle) const;
    std::vector<UIElement> getSiblings(const std::string& elementHandle) const;
};
```

## Validation Rules

### UIElement Validation

1. **Handle Uniqueness**: Element handles must be unique within the system
2. **Window Association**: Every element must have a valid parentWindowHandle
3. **Geometry Constraints**: Element position and size must be non-negative
4. **Type Consistency**: Element properties must be consistent with its type
5. **Hierarchy Integrity**: Parent-child relationships must be bidirectional

### ElementSearchQuery Validation

1. **Search Term**: Must not be empty for valid queries
2. **Type Filters**: Include and exclude type lists must not overlap
3. **Result Limits**: maxResults must be non-negative
4. **Field Consistency**: Search field must be appropriate for element types

### ElementEnumerationResult Validation

1. **Consistency**: totalElementCount must match elements.size()
2. **Window Reference**: windowHandle must not be empty for valid results
3. **Error State**: If success is false, errorMessage must not be empty
4. **Performance**: Operation should complete within reasonable time bounds

## State Transitions

### Element Discovery Lifecycle

```cpp
enum class ElementDiscoveryState {
    NotDiscovered,    // Element not yet found
    Discovering,      // Enumeration in progress
    Discovered,       // Element found and valid
    Stale,           // Element data may be outdated
    Invalid          // Element no longer exists
};
```

### Window-Element State Synchronization

1. **Window Active** → Elements can be enumerated
2. **Window Minimized** → Elements may have limited accessibility
3. **Window Closed** → All associated elements become invalid
4. **Window Focus Changed** → Element states may need refresh

## Cache Management

### Element Cache Strategy

```cpp
struct ElementCache {
    std::map<std::string, ElementEnumerationResult> windowCache; // Window -> elements
    std::map<std::string, std::chrono::steady_clock::time_point> cacheTimestamps; // Last update
    static constexpr std::chrono::milliseconds CACHE_TTL{30000}; // 30 second TTL

    // Cache operations
    void store(const std::string& windowHandle, const ElementEnumerationResult& result);
    std::optional<ElementEnumerationResult> retrieve(const std::string& windowHandle);
    void invalidate(const std::string& windowHandle);
    void cleanup(); // Remove expired entries
};
```

## Platform Abstraction

### Cross-Platform Element Mapping

Different platforms expose element information differently. The data model abstracts these differences:

```cpp
// Platform-specific element adapters
class PlatformElementAdapter {
public:
    virtual UIElement convertToUIElement(const PlatformSpecificElement& platformElement) = 0;
    virtual ElementType mapElementType(const PlatformSpecificType& platformType) = 0;
    virtual ElementState mapElementState(const PlatformSpecificState& platformState) = 0;
};

// Platform implementations
class MacOSElementAdapter : public PlatformElementAdapter {
    // Convert AXUIElement to UIElement
};

class WindowsElementAdapter : public PlatformElementAdapter {
    // Convert IUIAutomationElement to UIElement
};

class LinuxElementAdapter : public PlatformElementAdapter {
    // Convert AtspiAccessible to UIElement
};
```

This data model provides a robust foundation for cross-platform UI element enumeration while maintaining type safety, performance, and extensibility for future platform additions.