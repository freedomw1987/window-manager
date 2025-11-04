#include "element_query.hpp"
#include <algorithm>
#include <sstream>

namespace WindowManager {

// Default constructor
ElementSearchQuery::ElementSearchQuery()
    : field(ElementSearchField::All)
    , caseSensitive(false)
    , exactMatch(false)
    , includeHidden(false)
    , includeDisabled(false)
{
}

// Parameterized constructor
ElementSearchQuery::ElementSearchQuery(const std::string& term,
                                     ElementSearchField field,
                                     bool caseSensitive,
                                     bool exactMatch)
    : searchTerm(term)
    , field(field)
    , caseSensitive(caseSensitive)
    , exactMatch(exactMatch)
    , includeHidden(false)
    , includeDisabled(false)
{
}

// Validation methods
bool ElementSearchQuery::isValid() const {
    return !searchTerm.empty() || !typeFilter.empty();
}

bool ElementSearchQuery::hasTypeFilter() const {
    return !typeFilter.empty();
}

bool ElementSearchQuery::isEmpty() const {
    return searchTerm.empty() && typeFilter.empty();
}

// Matching logic
bool ElementSearchQuery::matches(const UIElement& element) const {
    // Check visibility and enabled state filters
    if (!matchesVisibility(element)) {
        return false;
    }

    // Check type filter
    if (!matchesType(element)) {
        return false;
    }

    // Check text search (if search term provided)
    if (!searchTerm.empty() && !matchesText(element)) {
        return false;
    }

    return true;
}

bool ElementSearchQuery::matchesType(const UIElement& element) const {
    if (typeFilter.empty()) {
        return true; // No type filter means all types match
    }

    return std::find(typeFilter.begin(), typeFilter.end(), element.type) != typeFilter.end();
}

bool ElementSearchQuery::matchesText(const UIElement& element) const {
    if (searchTerm.empty()) {
        return true; // No search term means text always matches
    }

    std::vector<std::string> searchFields;

    // Collect fields to search based on field setting
    switch (field) {
        case ElementSearchField::Name:
            searchFields.push_back(element.name);
            break;
        case ElementSearchField::Value:
            searchFields.push_back(element.value);
            break;
        case ElementSearchField::Description:
            searchFields.push_back(element.description);
            searchFields.push_back(element.accessibilityLabel);
            searchFields.push_back(element.accessibilityHelp);
            break;
        case ElementSearchField::Type:
            searchFields.push_back(elementTypeToString(element.type));
            break;
        case ElementSearchField::All:
        default:
            searchFields.push_back(element.name);
            searchFields.push_back(element.value);
            searchFields.push_back(element.description);
            searchFields.push_back(element.accessibilityLabel);
            searchFields.push_back(elementTypeToString(element.type));
            break;
    }

    // Search in collected fields
    std::string searchTermToUse = searchTerm;
    if (!caseSensitive) {
        std::transform(searchTermToUse.begin(), searchTermToUse.end(),
                      searchTermToUse.begin(), ::tolower);
    }

    for (const auto& field : searchFields) {
        if (field.empty()) continue;

        std::string fieldToSearch = field;
        if (!caseSensitive) {
            std::transform(fieldToSearch.begin(), fieldToSearch.end(),
                          fieldToSearch.begin(), ::tolower);
        }

        bool matches = false;
        if (exactMatch) {
            matches = (fieldToSearch == searchTermToUse);
        } else {
            matches = (fieldToSearch.find(searchTermToUse) != std::string::npos);
        }

        if (matches) {
            return true;
        }
    }

    return false;
}

bool ElementSearchQuery::matchesVisibility(const UIElement& element) const {
    // Check visibility filter
    if (!includeHidden && !element.isVisible) {
        return false;
    }

    // Check enabled state filter
    if (!includeDisabled && !element.isEnabled) {
        return false;
    }

    return true;
}

// Configuration methods
void ElementSearchQuery::addTypeFilter(ElementType type) {
    if (std::find(typeFilter.begin(), typeFilter.end(), type) == typeFilter.end()) {
        typeFilter.push_back(type);
    }
}

void ElementSearchQuery::clearTypeFilters() {
    typeFilter.clear();
}

void ElementSearchQuery::setSearchField(ElementSearchField field) {
    this->field = field;
}

// String representation
std::string ElementSearchQuery::toString() const {
    std::ostringstream oss;
    oss << "ElementSearchQuery{";

    if (!searchTerm.empty()) {
        oss << "term:\"" << searchTerm << "\", ";
        oss << "field:" << searchFieldToString(field) << ", ";
        oss << "caseSensitive:" << (caseSensitive ? "true" : "false") << ", ";
        oss << "exactMatch:" << (exactMatch ? "true" : "false");
    }

    if (!typeFilter.empty()) {
        if (!searchTerm.empty()) oss << ", ";
        oss << "types:[";
        for (size_t i = 0; i < typeFilter.size(); ++i) {
            if (i > 0) oss << ",";
            oss << elementTypeToString(typeFilter[i]);
        }
        oss << "]";
    }

    oss << ", includeHidden:" << (includeHidden ? "true" : "false");
    oss << ", includeDisabled:" << (includeDisabled ? "true" : "false");
    oss << "}";

    return oss.str();
}

std::string ElementSearchQuery::getDescription() const {
    std::ostringstream oss;

    if (!searchTerm.empty()) {
        oss << "Search for \"" << searchTerm << "\"";
        if (field != ElementSearchField::All) {
            oss << " in " << searchFieldToString(field);
        }
        if (caseSensitive) {
            oss << " (case-sensitive)";
        }
        if (exactMatch) {
            oss << " (exact match)";
        }
    }

    if (!typeFilter.empty()) {
        if (!searchTerm.empty()) oss << ", ";
        oss << "Filter by type: ";
        for (size_t i = 0; i < typeFilter.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << elementTypeToString(typeFilter[i]);
        }
    }

    return oss.str();
}

std::string ElementSearchQuery::getFilterSummary() const {
    std::ostringstream oss;

    if (includeHidden) {
        oss << "Including hidden elements. ";
    }
    if (includeDisabled) {
        oss << "Including disabled elements. ";
    }

    return oss.str();
}

// Utility functions for ElementSearchField
const char* searchFieldToString(ElementSearchField field) {
    switch (field) {
        case ElementSearchField::Name: return "Name";
        case ElementSearchField::Value: return "Value";
        case ElementSearchField::Description: return "Description";
        case ElementSearchField::Type: return "Type";
        case ElementSearchField::All: return "All";
        default: return "All";
    }
}

ElementSearchField stringToSearchField(const std::string& str) {
    if (str == "Name") return ElementSearchField::Name;
    if (str == "Value") return ElementSearchField::Value;
    if (str == "Description") return ElementSearchField::Description;
    if (str == "Type") return ElementSearchField::Type;
    if (str == "All") return ElementSearchField::All;
    return ElementSearchField::All;
}

} // namespace WindowManager