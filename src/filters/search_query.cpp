#include "search_query.hpp"
#include "../core/window.hpp"
#include <algorithm>
#include <cctype>
#include <regex>
#include <sstream>

namespace WindowManager {

// T031: Implement SearchQuery validation and matching logic

SearchQuery::SearchQuery()
    : query("")
    , field(SearchField::Both)
    , caseSensitive(false)
    , useRegex(false)
    , workspaceFilter("")
    , matchMode(MatchMode::CONTAINS)
    , timestamp(std::chrono::steady_clock::now()) {
}

SearchQuery::SearchQuery(const std::string& query, SearchField field,
                        bool caseSensitive, bool useRegex)
    : query(query)
    , field(field)
    , caseSensitive(caseSensitive)
    , useRegex(useRegex)
    , workspaceFilter("")
    , matchMode(useRegex ? MatchMode::REGEX : MatchMode::CONTAINS)
    , timestamp(std::chrono::steady_clock::now()) {
}

bool SearchQuery::matches(const WindowInfo& window) const {
    if (isEmpty()) {
        return true; // Empty query matches all windows
    }

    // Check workspace filter first
    if (!workspaceFilter.empty() && window.workspaceId != workspaceFilter) {
        return false;
    }

    // Determine which fields to search based on the field setting
    bool searchTitle = (field == SearchField::Title || field == SearchField::Both);
    bool searchOwner = (field == SearchField::Owner || field == SearchField::Both);

    bool titleMatch = false;
    bool ownerMatch = false;

    if (searchTitle) {
        titleMatch = matchesTitle(window.title);
    }

    if (searchOwner) {
        ownerMatch = matchesOwner(window.ownerName);
    }

    // For field "Both", return true if either title or owner matches
    // For specific fields, return the specific match result
    switch (field) {
        case SearchField::Title:
            return titleMatch;
        case SearchField::Owner:
            return ownerMatch;
        case SearchField::Both:
        default:
            return titleMatch || ownerMatch;
    }
}

// T032: Add SearchQuery title and owner matching methods

bool SearchQuery::matchesTitle(const std::string& title) const {
    return performStringMatch(title, query);
}

bool SearchQuery::matchesOwner(const std::string& owner) const {
    return performStringMatch(owner, query);
}

bool SearchQuery::isEmpty() const {
    return query.empty();
}

bool SearchQuery::isValid() const {
    // Check query length constraint (max 1000 characters as per data model)
    if (query.length() > 1000) {
        return false;
    }

    // For regex mode, validate the pattern
    if (useRegex && !query.empty()) {
        try {
            std::regex pattern(query, caseSensitive ? std::regex::ECMAScript : std::regex::icase);
            return true;
        } catch (const std::regex_error&) {
            return false;
        }
    }

    return true;
}

// Helper method for string matching
bool SearchQuery::performStringMatch(const std::string& text, const std::string& searchQuery) const {
    if (searchQuery.empty()) {
        return true;
    }

    std::string targetText = text;
    std::string searchTerm = searchQuery;

    // Convert to lowercase for case-insensitive matching
    if (!caseSensitive) {
        std::transform(targetText.begin(), targetText.end(), targetText.begin(),
                      [](unsigned char c) { return std::tolower(c); });
        std::transform(searchTerm.begin(), searchTerm.end(), searchTerm.begin(),
                      [](unsigned char c) { return std::tolower(c); });
    }

    if (useRegex) {
        try {
            std::regex pattern(searchTerm, caseSensitive ? std::regex::ECMAScript : std::regex::icase);
            return std::regex_search(targetText, pattern);
        } catch (const std::regex_error&) {
            // If regex is invalid, fall back to contains match
            return targetText.find(searchTerm) != std::string::npos;
        }
    } else {
        // Default to contains matching
        return targetText.find(searchTerm) != std::string::npos;
    }
}

std::string SearchQuery::toString() const {
    std::ostringstream oss;
    oss << "SearchQuery{";
    oss << "query='" << query << "', ";
    oss << "field=";

    switch (field) {
        case SearchField::Title:
            oss << "Title";
            break;
        case SearchField::Owner:
            oss << "Owner";
            break;
        case SearchField::Both:
            oss << "Both";
            break;
    }

    oss << ", caseSensitive=" << (caseSensitive ? "true" : "false");
    oss << ", useRegex=" << (useRegex ? "true" : "false");

    if (!workspaceFilter.empty()) {
        oss << ", workspaceFilter='" << workspaceFilter << "'";
    }

    oss << "}";
    return oss.str();
}

} // namespace WindowManager