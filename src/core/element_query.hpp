#pragma once

#include <string>
#include <vector>
#include "element_types.hpp"
#include "ui_element.hpp"

namespace WindowManager {

/**
 * Enumeration of search fields for element queries
 */
enum class ElementSearchField {
    Name,
    Value,
    Description,
    Type,
    All
};

/**
 * Search criteria for filtering elements within windows
 * Defines how to search and filter UI elements based on various criteria
 */
struct ElementSearchQuery {
    std::string searchTerm;
    ElementSearchField field;
    bool caseSensitive;
    bool exactMatch;
    std::vector<ElementType> typeFilter;  // Filter by element types
    bool includeHidden;                   // Include hidden elements
    bool includeDisabled;                 // Include disabled elements

    // Constructor
    ElementSearchQuery();
    ElementSearchQuery(const std::string& term,
                      ElementSearchField field = ElementSearchField::All,
                      bool caseSensitive = false,
                      bool exactMatch = false);

    // Validation methods
    bool isValid() const;
    bool hasTypeFilter() const;
    bool isEmpty() const;

    // Matching logic
    bool matches(const UIElement& element) const;
    bool matchesType(const UIElement& element) const;
    bool matchesText(const UIElement& element) const;
    bool matchesVisibility(const UIElement& element) const;

    // Configuration methods
    void addTypeFilter(ElementType type);
    void clearTypeFilters();
    void setSearchField(ElementSearchField field);

    // String representation
    std::string toString() const;
    std::string getDescription() const;
    std::string getFilterSummary() const;
};

/**
 * Utility functions for ElementSearchField
 */
const char* searchFieldToString(ElementSearchField field);
ElementSearchField stringToSearchField(const std::string& str);

} // namespace WindowManager