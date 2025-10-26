#pragma once

#include <vector>
#include <map>
#include <chrono>
#include "../core/window.hpp"
#include "../core/workspace.hpp"
#include "search_query.hpp"

namespace WindowManager {

// T044: Cross-workspace statistics structure
struct WorkspaceStatistics {
    size_t totalWorkspaces = 0;
    size_t totalWindows = 0;
    size_t activeWorkspaces = 0;
    size_t visibleWindows = 0;
    size_t minimizedWindows = 0;
    size_t focusedWindows = 0;
    size_t hiddenWindows = 0;
    double averageWindowsPerWorkspace = 0.0;
    std::map<std::string, size_t> windowsByWorkspace;
};

/**
 * Filter operation result with workspace grouping support
 * Contains filtered windows, workspace grouping, and performance metrics
 */
struct FilterResult {
    std::vector<WindowInfo> windows;
    size_t totalCount;
    size_t filteredCount;
    std::chrono::milliseconds searchTime;
    SearchQuery query;

    // T033: Enhanced workspace grouping support
    std::vector<WorkspaceInfo> workspaces;
    std::map<std::string, std::vector<WindowInfo>> windowsByWorkspace;
    std::map<std::string, size_t> windowCountsByWorkspace;

    // Constructors
    FilterResult() = default;
    FilterResult(std::vector<WindowInfo> windows, size_t total,
                const SearchQuery& query, std::chrono::milliseconds time);

    // T033: Enhanced constructor with workspace information
    FilterResult(std::vector<WindowInfo> windows, size_t total,
                const SearchQuery& query, std::chrono::milliseconds time,
                const std::vector<WorkspaceInfo>& workspaces);

    // Result analysis
    double filterRatio() const;
    bool meetsPerformanceTarget() const; // <1 second requirement
    std::string getSummary() const;

    // T034: Workspace grouping and statistics
    void groupByWorkspace();
    size_t getWorkspaceCount() const;
    std::vector<std::string> getWorkspaceIds() const;
    size_t getWindowCountForWorkspace(const std::string& workspaceId) const;
    std::string getWorkspaceStatsSummary() const;

    // Validation
    bool isValid() const;

    // T035: Enhanced output formatting with workspace info
    std::string toJson() const;
    std::string toJsonWithWorkspaces() const;

    // T044: Cross-workspace window statistics
    std::string getCrossWorkspaceStatistics() const;
    WorkspaceStatistics getWorkspaceStatistics() const;
    size_t getVisibleWindowCount() const;
    size_t getMinimizedWindowCount() const;
    size_t getFocusedWindowCount() const;
    size_t getActiveWorkspaceCount() const;
    double getAverageWindowsPerWorkspace() const;
    std::map<std::string, size_t> getWorkspaceDistribution() const;
};

} // namespace WindowManager