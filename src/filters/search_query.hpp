#pragma once

#include <string>
#include <chrono>

namespace WindowManager {

struct WindowInfo; // Forward declaration

/**
 * Search field enumeration for enhanced search functionality
 * Determines which fields to search in window information
 */
enum class SearchField {
    Title,      // Search in window title
    Owner,      // Search in application/owner name
    Both        // Search in both title and owner (default)
};

/**
 * Enhanced search query supporting multiple fields
 * Backward compatible with existing single-field searches
 */
struct SearchQuery {
    enum class MatchMode {
        CONTAINS,       // Substring search (default)
        STARTS_WITH,    // Prefix matching
        EXACT,          // Exact match
        REGEX           // Regular expression (optional)
    };

    std::string query;                    // Search term
    SearchField field;                    // Which field(s) to search
    bool caseSensitive;                   // Case sensitivity flag
    bool useRegex;                        // Regular expression mode
    std::string workspaceFilter;          // Filter by workspace ID (empty = all)
    MatchMode matchMode = MatchMode::CONTAINS;
    std::chrono::steady_clock::time_point timestamp;

    // Constructors
    SearchQuery();
    explicit SearchQuery(const std::string& query, SearchField field = SearchField::Both,
                        bool caseSensitive = false, bool useRegex = false);

    // Query operations
    bool matches(const WindowInfo& window) const;
    bool matchesTitle(const std::string& title) const;
    bool matchesOwner(const std::string& owner) const;
    bool isEmpty() const;

    // Validation and utility methods
    bool isValid() const;
    std::string toString() const;

private:
    // Helper method for string matching
    bool performStringMatch(const std::string& text, const std::string& searchQuery) const;
};

} // namespace WindowManager