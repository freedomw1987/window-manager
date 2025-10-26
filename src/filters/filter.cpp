#include "filter.hpp"
#include <algorithm>
#include <chrono>
#include <functional>
#include <sstream>

namespace WindowManager {

// WindowFilter base class convenience methods

// T036: Enhance existing filter logic to use SearchQuery

FilterResult WindowFilter::filterByKeyword(const std::vector<WindowInfo>& windows,
                                          const std::string& keyword) {
    SearchQuery query(keyword, SearchField::Both, false, false);
    return filter(windows, query);
}

FilterResult WindowFilter::filterVisible(const std::vector<WindowInfo>& windows) {
    auto startTime = std::chrono::steady_clock::now();

    std::vector<WindowInfo> visibleWindows;
    std::copy_if(windows.begin(), windows.end(), std::back_inserter(visibleWindows),
                [](const WindowInfo& window) { return window.isVisible; });

    auto endTime = std::chrono::steady_clock::now();
    auto searchTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    SearchQuery emptyQuery; // Empty query for "show all visible"
    return FilterResult(std::move(visibleWindows), windows.size(), emptyQuery, searchTime);
}

std::unique_ptr<WindowFilter> WindowFilter::create() {
    return std::make_unique<WindowFilterImpl>();
}

// WindowFilterImpl implementation

WindowFilterImpl::WindowFilterImpl()
    : cachingEnabled_(true)
    , cacheHits_(0)
    , cacheRequests_(0) {
}

FilterResult WindowFilterImpl::filter(const std::vector<WindowInfo>& windows,
                                     const SearchQuery& query) {
    updateCacheStats();

    // Check cache first if enabled
    if (cachingEnabled_) {
        std::string cacheKey = generateCacheKey(windows, query);
        auto cacheIt = cache_.find(cacheKey);
        if (cacheIt != cache_.end()) {
            ++cacheHits_;
            return cacheIt->second;
        }
    }

    // Perform actual filtering
    FilterResult result = performFilter(windows, query);

    // Store in cache if enabled
    if (cachingEnabled_) {
        std::string cacheKey = generateCacheKey(windows, query);
        cache_[cacheKey] = result;
    }

    return result;
}

FilterResult WindowFilterImpl::filterWithWorkspaces(const std::vector<WindowInfo>& windows,
                                                   const SearchQuery& query,
                                                   const std::vector<WorkspaceInfo>& workspaces) {
    updateCacheStats();

    // Use the same cache logic but include workspace info in the result
    FilterResult result = performFilter(windows, query);

    // Create enhanced result with workspace information
    FilterResult enhancedResult(result.windows, result.totalCount, result.query, result.searchTime, workspaces);

    return enhancedResult;
}

void WindowFilterImpl::setCaching(bool enabled) {
    cachingEnabled_ = enabled;
    if (!enabled) {
        clearCache();
    }
}

void WindowFilterImpl::clearCache() {
    cache_.clear();
    cacheHits_ = 0;
    cacheRequests_ = 0;
}

size_t WindowFilterImpl::getCacheSize() const {
    return cache_.size();
}

double WindowFilterImpl::getCacheHitRatio() const {
    if (cacheRequests_ == 0) {
        return 0.0;
    }
    return static_cast<double>(cacheHits_) / static_cast<double>(cacheRequests_);
}

std::string WindowFilterImpl::generateCacheKey(const std::vector<WindowInfo>& windows,
                                              const SearchQuery& query) const {
    std::ostringstream oss;

    // Include window count and query details in cache key
    oss << "count:" << windows.size();
    oss << "|query:" << query.query;
    oss << "|field:" << static_cast<int>(query.field);
    oss << "|case:" << (query.caseSensitive ? "1" : "0");
    oss << "|regex:" << (query.useRegex ? "1" : "0");
    oss << "|workspace:" << query.workspaceFilter;

    // Hash of window titles and owners for cache invalidation
    std::hash<std::string> hasher;
    size_t contentHash = 0;
    for (const auto& window : windows) {
        contentHash ^= hasher(window.title) + 0x9e3779b9 + (contentHash << 6) + (contentHash >> 2);
        contentHash ^= hasher(window.ownerName) + 0x9e3779b9 + (contentHash << 6) + (contentHash >> 2);
        contentHash ^= hasher(window.workspaceId) + 0x9e3779b9 + (contentHash << 6) + (contentHash >> 2);
    }
    oss << "|hash:" << contentHash;

    return oss.str();
}

FilterResult WindowFilterImpl::performFilter(const std::vector<WindowInfo>& windows,
                                            const SearchQuery& query) const {
    auto startTime = std::chrono::steady_clock::now();

    std::vector<WindowInfo> filteredWindows;
    // Performance optimization: Reserve space based on expected filter ratio
    if (query.isEmpty()) {
        filteredWindows.reserve(windows.size());
    } else {
        // Assume 10-20% match rate for keyword searches
        filteredWindows.reserve(windows.size() / 5);
    }

    // If query is empty, return all visible windows
    if (query.isEmpty()) {
        std::copy_if(windows.begin(), windows.end(), std::back_inserter(filteredWindows),
                    [](const WindowInfo& window) { return window.isVisible; });
    } else {
        // Filter based on query criteria - include windows from all workspaces
        std::copy_if(windows.begin(), windows.end(), std::back_inserter(filteredWindows),
                    [&query](const WindowInfo& window) {
                        // T036: Enhanced cross-workspace search - don't filter by visibility
                        // Include windows from all workspaces, not just visible ones
                        return query.matches(window);
                    });
    }

    // Sort results for consistent presentation
    // Performance optimization: Skip sorting for very large result sets
    if (filteredWindows.size() <= 1000) {
        std::sort(filteredWindows.begin(), filteredWindows.end(),
                 [](const WindowInfo& a, const WindowInfo& b) {
                     // Sort by title first, then by process ID
                     if (a.title != b.title) {
                         return a.title < b.title;
                     }
                     return a.processId < b.processId;
                 });
    }

    auto endTime = std::chrono::steady_clock::now();
    auto searchTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    return FilterResult(std::move(filteredWindows), windows.size(), query, searchTime);
}

void WindowFilterImpl::updateCacheStats() const {
    ++cacheRequests_;
}

} // namespace WindowManager