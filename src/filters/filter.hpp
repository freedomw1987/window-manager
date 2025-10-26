#pragma once

#include <vector>
#include <memory>
#include <unordered_map>
#include <string>
#include "../core/window.hpp"
#include "../core/workspace.hpp"
#include "search_query.hpp"
#include "filter_result.hpp"

namespace WindowManager {

/**
 * Window filter interface
 * Handles search and filtering operations
 */
class WindowFilter {
public:
    virtual ~WindowFilter() = default;

    // Core filtering operations
    virtual FilterResult filter(const std::vector<WindowInfo>& windows,
                               const SearchQuery& query) = 0;

    // T036: Enhanced filtering with workspace information
    virtual FilterResult filterWithWorkspaces(const std::vector<WindowInfo>& windows,
                                             const SearchQuery& query,
                                             const std::vector<WorkspaceInfo>& workspaces) = 0;

    // Convenience methods
    FilterResult filterByKeyword(const std::vector<WindowInfo>& windows,
                                const std::string& keyword);
    FilterResult filterVisible(const std::vector<WindowInfo>& windows);

    // Performance optimization
    virtual void setCaching(bool enabled) = 0;
    virtual void clearCache() = 0;

    // Factory method
    static std::unique_ptr<WindowFilter> create();
};

/**
 * Concrete implementation of WindowFilter
 * Provides efficient filtering with caching support
 */
class WindowFilterImpl : public WindowFilter {
public:
    WindowFilterImpl();
    ~WindowFilterImpl() override = default;

    // Non-copyable, moveable
    WindowFilterImpl(const WindowFilterImpl&) = delete;
    WindowFilterImpl& operator=(const WindowFilterImpl&) = delete;
    WindowFilterImpl(WindowFilterImpl&&) = default;
    WindowFilterImpl& operator=(WindowFilterImpl&&) = default;

    // Core filtering implementation
    FilterResult filter(const std::vector<WindowInfo>& windows,
                       const SearchQuery& query) override;

    // Enhanced filtering with workspace information
    FilterResult filterWithWorkspaces(const std::vector<WindowInfo>& windows,
                                     const SearchQuery& query,
                                     const std::vector<WorkspaceInfo>& workspaces) override;

    // Performance optimization
    void setCaching(bool enabled) override;
    void clearCache() override;

    // Statistics and diagnostics
    size_t getCacheSize() const;
    double getCacheHitRatio() const;

private:
    bool cachingEnabled_;
    std::unordered_map<std::string, FilterResult> cache_;
    mutable size_t cacheHits_;
    mutable size_t cacheRequests_;

    // Helper methods
    std::string generateCacheKey(const std::vector<WindowInfo>& windows,
                                const SearchQuery& query) const;
    FilterResult performFilter(const std::vector<WindowInfo>& windows,
                              const SearchQuery& query) const;
    void updateCacheStats() const;
};

} // namespace WindowManager